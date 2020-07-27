#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

bool mqtt_init(char *server, uint16_t port, char *user, char *password, char *topic_prefix, int qos_level);
void mqtt_destroy();
bool mqtt_send(char *topic, char *payload);

#endif