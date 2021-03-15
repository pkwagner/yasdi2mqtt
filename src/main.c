#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <log.h>
#include <cjson/cJSON.h>
#include "yasdi_handler.h"
#include "mqtt_client.h"

void new_values_cb(struct device_value_t *values);

int main(int argc, char **argv)
{
    log_set_level(LOG_INFO);

    int mqtt_qos_level = 2;
    uint16_t mqtt_port = 0;
    unsigned int yasdi_update_interval = 0;
    DWORD yasdi_driver_id = 0, yasdi_max_device_count = 0;
    char *yasdi_config = NULL, *mqtt_topic_prefix = NULL, *mqtt_server = NULL, *mqtt_user = NULL, *mqtt_password = NULL,
         *ssl_cert = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "c:d:i:u:t:s:p:q:U:P:l:S:")) != -1)
    {
        switch (opt)
        {
        case 'c':
            yasdi_config = optarg;
            break;
        case 'd':
            yasdi_driver_id = strtoul(optarg, NULL, 10);
            break;
        case 'i':
            yasdi_max_device_count = strtoul(optarg, NULL, 10);
            break;
        case 'u':
            yasdi_update_interval = strtoul(optarg, NULL, 10);
            break;
        case 't':
            mqtt_topic_prefix = optarg;
            break;
        case 's':
            mqtt_server = optarg;
            break;
        case 'p':
            mqtt_port = strtoul(optarg, NULL, 10);
            break;
        case 'q':
            mqtt_qos_level = strtol(optarg, NULL, 10);
            break;
        case 'U':
            mqtt_user = optarg;
            break;
        case 'P':
            mqtt_password = optarg;
            break;
        case 'l':
            log_set_level(strtol(optarg, NULL, 10));
            break;
        case 'S':
            ssl_cert = optarg;
            break;
        case '?':
            return -1;
        }
    }

    log_info("Configuration | yasdi_config = %s", yasdi_config);
    log_info("Configuration | yasdi_driver_id = %u", yasdi_driver_id);
    log_info("Configuration | yasdi_max_device_count = %u", yasdi_max_device_count);
    log_info("Configuration | yasdi_update_interval = %u", yasdi_update_interval);
    log_info("Configuration | mqtt_topic_prefix = %s", mqtt_topic_prefix);
    log_info("Configuration | mqtt_server = %s", mqtt_server);
    log_info("Configuration | mqtt_port = %u", mqtt_port);
    log_info("Configuration | mqtt_qos_level = %d", mqtt_qos_level);
    log_info("Configuration | mqtt_user = %s", mqtt_user);
    log_info("Configuration | mqtt_password = %s", mqtt_password);
    log_info("Configuration | ssl_cert = %s", ssl_cert);

    if (yasdi_config == NULL || mqtt_topic_prefix == NULL || mqtt_server == NULL || yasdi_max_device_count == 0 || yasdi_update_interval == 0 || mqtt_port == 0)
    {
        printf("\nToo few arguments. See README.md for further assistance.\n");
        printf("Usage: yasdi2mqtt -c <yasdi_config> -d <yasdi_driver_id> -i <yasdi_max_device_count> -u <yasdi_update_interval> -t <mqtt_topic_prefix> -s <mqtt_server> -p <mqtt_port> (-q <mqtt_qos_level>) (-U <mqtt_user>) (-P <mqtt_password>) (-l <log_level>)\n");
        return -1;
    }

    if (!mqtt_init(mqtt_server, mqtt_port, mqtt_user, mqtt_password, mqtt_topic_prefix, mqtt_qos_level, ssl_cert))
    {
        log_fatal("Unable to initialize mqtt_client");
        return -1;
    }

    if (!yh_init(yasdi_config, yasdi_driver_id, yasdi_max_device_count, yasdi_update_interval))
    {
        log_fatal("Unable to initialize yasdi_handler");
        return -1;
    }

    // Run value (and device) detection in endless loop, will notify about new values via cb
    yh_new_values_cb = new_values_cb;
    yh_loop();

    return 0;
}

void new_values_cb(struct device_value_t *values)
{
    char topic[11];
    sprintf(topic, "%u", values->device_sn);
    char *json = cJSON_PrintUnformatted(values->json);

    mqtt_send(topic, json);
    free(json);
}