#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <log.h>
#include <cjson/cJSON.h>
#include "yasdi_handler.h"
#include "mqtt_client.h"

char *get_required_env(const char *name);
void new_values_cb(struct device_value_t *values);

int main(int argc, char **argv)
{
    log_set_level(LOG_INFO);

    char *yasdi_config = get_required_env("YASDI_CONFIG");
    DWORD yasdi_driver_id = strtoul(get_required_env("YASDI_DRIVER_ID"), NULL, 10);
    DWORD yasdi_max_device_count = strtoul(get_required_env("YASDI_MAX_DEVICE_COUNT"), NULL, 10);
    unsigned int yasdi_update_interval = strtoul(get_required_env("YASDI_UPDATE_INTERVAL"), NULL, 10);
    char *mqtt_topic_prefix = get_required_env("MQTT_TOPIC_PREFIX");
    char *mqtt_server = get_required_env("MQTT_SERVER");
    uint16_t mqtt_port = strtoul(get_required_env("MQTT_PORT"), NULL, 10);
    char *mqtt_ssl_cert = getenv("MQTT_SSL_CERT");
    char *mqtt_user = getenv("MQTT_USER");
    char *mqtt_password = getenv("MQTT_PASSWORD");

    char *mqtt_client_id;
    if (getenv("MQTT_CLIENT_ID"))
    {
      mqtt_client_id = getenv("MQTT_CLIENT_ID");
    } else {
      mqtt_client_id = "yasdi2mqtt";
    }

    int mqtt_qos_level = 2;
    if (getenv("MQTT_QOS_LEVEL"))
    {
        mqtt_qos_level = strtol(getenv("MQTT_QOS_LEVEL"), NULL, 10);
    }

    if (getenv("LOG_LEVEL"))
    {
        log_set_level(strtol(getenv("LOG_LEVEL"), NULL, 10));
    }

    log_info("Configuration | YASDI_CONFIG = %s", yasdi_config);
    log_info("Configuration | YASDI_DRIVER_ID = %u", yasdi_driver_id);
    log_info("Configuration | YASDI_MAX_DEVICE_COUNT = %u", yasdi_max_device_count);
    log_info("Configuration | YASDI_UPDATE_INTERVAL = %u", yasdi_update_interval);
    log_info("Configuration | MQTT_TOPIC_PREFIX = %s", mqtt_topic_prefix);
    log_info("Configuration | MQTT_SERVER = %s", mqtt_server);
    log_info("Configuration | MQTT_PORT = %u", mqtt_port);
    log_info("Configuration | MQTT_SSL_CERT = %s", mqtt_ssl_cert);
    log_info("Configuration | MQTT_QOS_LEVEL = %d", mqtt_qos_level);
    log_info("Configuration | MQTT_USER = %s", mqtt_user);
    log_info("Configuration | MQTT_PASSWORD = %s", mqtt_password);
    log_info("Configuration | MQTT_CLIENT_ID = %s", mqtt_client_id);

    if (!mqtt_init(mqtt_server, mqtt_port, mqtt_ssl_cert, mqtt_user, mqtt_password, mqtt_topic_prefix, mqtt_qos_level, mqtt_client_id))
    {
        log_fatal("Unable to initialize mqtt_client");
        return -1;
    }

    if (!yh_init(yasdi_config, yasdi_driver_id, yasdi_max_device_count, yasdi_update_interval))
    {
        log_fatal("Unable to initialize yasdi_handler");
        return -1;
    }

    yh_new_values_cb = new_values_cb;
    yh_loop();

    return 0;
}

char *get_required_env(const char *name)
{
    if (!getenv(name))
    {
        log_fatal("Mandatory environment variable %s missing", name);
        exit(-1);
    }

    return getenv(name);
}

void new_values_cb(struct device_value_t *values)
{
    char topic[11];
    sprintf(topic, "%u", values->device_sn);
    char *json = cJSON_PrintUnformatted(values->json);

    mqtt_send(topic, json);
    free(json);
}
