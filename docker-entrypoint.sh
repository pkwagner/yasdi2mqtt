#!/bin/sh
set -e

# TODO Make username/password optional
# TODO Pass debug level
yasdi2mqtt -c "$YASDI_CONFIG" -d "$YASDI_DRIVER_ID" -i "$YASDI_MAX_DEVICE_COUNT" -u "$YASDI_UPDATE_INTERVAL" -t "$MQTT_TOPIC_PREFIX" -s "$MQTT_SERVER" -p "$MQTT_PORT" -U "$MQTT_USER" -P "$MQTT_PASSWORD"