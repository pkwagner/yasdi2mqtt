#ifndef YASDI_HANDLER_H
#define YASDI_HANDLER_H

#include <stdbool.h>
#include <cjson/cJSON.h>
#include <smadef.h>

/**
 * Represents a value snapshot of a single device.
 */
struct device_value_t
{
    DWORD device_sn;
    cJSON *json;
};

extern void (*yh_new_values_cb)(struct device_value_t *);

bool yh_init(const char *ini_file, DWORD driver_id, DWORD device_count, unsigned int device_update_interval);
void yh_destroy();
void yh_loop();

#endif