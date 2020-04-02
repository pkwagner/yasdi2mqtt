#include "yasdi_handler.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <log.h>
#include <libyasdimaster.h>

#define MAX_CHANNEL_COUNT 300
#define MAX_VALUE_STR_SIZE 10
#define MAX_CHANNEL_NAME_SIZE 20

DWORD driver;
DWORD max_device_count;
DWORD *active_devices;
unsigned int update_interval;

void (*yh_new_values_cb)(struct device_value_t *);

struct device_value_t **yh_get_values();
void device_detection_cb(TYASDIDetectionSub event, DWORD device);

/**
 * Initializes yasdiMaster, enables the driver and prepares for device detection. Must be called first.
 * @param ini_file               relative location of yasdi.ini file
 * @param driver_id              id of the yasdi.ini driver to use (counts from 0)
 * @param device_countmaximum    amount of devices that might be connected at the same time
 * @param device_update_interval seconds to wait between device value updates
 * @return true if initialization succeeded
 */
bool yh_init(const char *ini_file, DWORD driver_id, DWORD device_count, unsigned int device_update_interval)
{
    int status;
    max_device_count = device_count;
    update_interval = device_update_interval;

    DWORD driver_count;
    status = yasdiMasterInitialize(ini_file, &driver_count);
    if (status != 0)
    {
        log_fatal("Error while initializing yasdiMaster: %d", status);
        return false;
    }
    else if (driver_count <= driver_id)
    {
        log_fatal("Selected driver id %u does not exist (%u drivers found)", driver_id, driver_count);
        return false;
    }

    DWORD drivers[driver_count];
    yasdiMasterGetDriver(drivers, driver_count);
    driver = drivers[driver_id];

    if (!yasdiMasterSetDriverOnline(driver))
    {
        log_fatal("Unable to set driver id %u online", driver);
        return false;
    }

    // Prepare for device detection
    active_devices = calloc(max_device_count, sizeof(DWORD));
    yasdiMasterAddEventListener(device_detection_cb, YASDI_EVENT_DEVICE_DETECTION);

    return true;
}

/**
 * Shuts down yasdiMaster and frees related resources.
 */
void yh_destroy()
{
    yasdiMasterRemEventListener(device_detection_cb, YASDI_EVENT_DEVICE_DETECTION);
    yasdiMasterSetDriverOffline(driver);
    yasdiMasterShutdown();

    free(active_devices);
}

/**
 * TODO
 */
void yh_loop()
{
    for (;;)
    {
        // Count active devices
        DWORD active_device_count = 0;
        for (DWORD i = 0; i < max_device_count; i++)
        {
            if (active_devices[i] != 0)
                active_device_count++;
        }

        // Check for new devices if not all are online yet
        if (active_device_count != max_device_count)
        {
            log_debug("Not all devices are online, starting device detection (async)...");

            int status = DoStartDeviceDetection(max_device_count, false);
            switch (status)
            {
            case YE_OK:
                break;
            case YE_NOT_ALL_DEVS_FOUND:
                log_debug("Unable to find all devices (target: %u)", max_device_count);
                break;
            case YE_DEV_DETECT_IN_PROGRESS:
                log_debug("Device detection already running, won't start new query");
                break;
            default:
                log_error("Unknown error while detecting devices: %d", status);
            }
        }

        // Collect inverter data and call cb for each
        struct device_value_t **values = yh_get_values();
        for (DWORD i = 0; i < max_device_count; i++)
        {
            struct device_value_t *value = values[i];

            if (value != NULL)
            {
                (*yh_new_values_cb)(value);

                cJSON_Delete(value->json);
                free(value);
            }
        }
        free(values);

        // TODO Subtract execution time
        log_debug("yh_loop is going to sleep for %u seconds...", update_interval);
        sleep(update_interval);
    }
}

struct device_value_t **yh_get_values()
{
    int status;
    struct device_value_t **values = calloc(max_device_count, sizeof(struct device_value_t *));

    log_debug("Starting device value fetch (sync)...");
    for (DWORD device_i = 0; device_i < max_device_count; device_i++)
    {
        DWORD device = active_devices[device_i];
        if (device == 0)
            continue;

        DWORD channels[MAX_CHANNEL_COUNT];
        DWORD channel_count = GetChannelHandlesEx(device, channels, MAX_CHANNEL_COUNT, SPOTCHANNELS);
        if (channel_count == 0)
        {
            log_debug("Device channel list download is apparently still in progress, skipping device %u...", device);
            continue;
        }

        DWORD device_sn;
        status = GetDeviceSN(device, &device_sn);
        switch (status)
        {
        case YE_OK:
            break;
        case YE_UNKNOWN_HANDLE:
            log_error("Unable to resolve sn of device %u", device);
            continue;
        default:
            log_error("Unknown error while detecting sn of device %u", device);
            continue;
        }

        // Prepare json output (format: {"sn": "...", "values": {...}})
        cJSON *device_json = cJSON_CreateObject();
        cJSON_AddNumberToObject(device_json, "sn", device_sn);
        cJSON *device_values = cJSON_CreateObject();
        cJSON_AddItemToObject(device_json, "values", device_values);

        for (DWORD channel_i = 0; channel_i < channel_count; channel_i++)
        {
            DWORD channel = channels[channel_i];

            char channel_name[MAX_CHANNEL_NAME_SIZE];
            status = GetChannelName(channel, channel_name, MAX_CHANNEL_NAME_SIZE);
            switch (status)
            {
            case YE_OK:
                break;
            case YE_UNKNOWN_HANDLE:
                log_error("Unable to resolve name for device %u / channel %u", device, channel);
                continue;
            default:
                log_error("Unknown error while resolving name for device %u / channel %u: %d", device, channel, status);
                continue;
            }

            double value;
            char value_str[MAX_VALUE_STR_SIZE];
            status = GetChannelValue(channel, device, &value, value_str, MAX_VALUE_STR_SIZE, 0);
            switch (status)
            {
            case YE_OK:
                break;
            case YE_UNKNOWN_HANDLE:
                log_error("Unknown channel %u for device %u", channel, device);
                continue;
            case YE_TIMEOUT:
                // Device apparently went offline
                log_debug("Timeout for device %u (channel %u) while fetching value", device, channel);
                RemoveDevice(device);
                goto skip_device;
            case YE_VALUE_NOT_VALID:
                log_error("Got invalid value for device %u / channel %u", device, channel);
                continue;
            default:
                log_error("Unknown error while fetching value for device %u / channel %u: %d", device, channel, status);
                continue;
            }

            if (value_str[0] == '\0')
                cJSON_AddNumberToObject(device_values, channel_name, value);
            else
                cJSON_AddStringToObject(device_values, channel_name, value_str);
        }

        // Add sn and device json (including values) to array
        values[device_i] = malloc(sizeof(struct device_value_t));
        values[device_i]->device_sn = device_sn;
        values[device_i]->json = device_json;

        continue;

    skip_device:;
        // Clean up device json object if not added to array
        cJSON_Delete(device_json);
    }

    return values;
}

void device_detection_cb(TYASDIDetectionSub event, DWORD device)
{
    log_debug("Event %d fired for device %u", event, device);

    // Check if device is already active & find any free index if possible
    DWORD device_i, free_item_i;
    bool device_active = false, has_free_item = false;
    for (DWORD i = 0; i < max_device_count; i++)
    {
        if (active_devices[i] == device)
        {
            // Device is already active
            device_i = i;
            device_active = true;
        }
        else if (active_devices[i] == 0)
        {
            // Found free index
            free_item_i = i;
            has_free_item = true;
        }
    }

    switch (event)
    {
    case YASDI_EVENT_DEVICE_ADDED:
        log_info("Device %u went online", device);
        if (!device_active)
        {
            if (has_free_item)
                active_devices[free_item_i] = device;
            else
                log_error("Device count exceeds active_devices array size (size: %u)", max_device_count);
        }
        else
        {
            log_warn("Detected device %u is already part of active_devices", device);
        }
        break;
    case YASDI_EVENT_DEVICE_REMOVED:
        log_info("Device %u went offline", device);
        if (device_active)
            active_devices[device_i] = 0;
        else
            log_warn("Removed device %u is not part of active_devices", device);
        break;
    case YASDI_EVENT_DEVICE_SEARCH_END:
        log_debug("Device detection finished with %u devices", device);
        break;
    case YASDI_EVENT_DOWNLOAD_CHANLIST:
        log_debug("Downloading channel list for device %u", device);
        break;
    default:
        log_warn("Got invalid event (device: %u) for device_detection_cb: %d", device, event);
    }
}
