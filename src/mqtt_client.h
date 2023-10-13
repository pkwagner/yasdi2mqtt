#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

bool mqtt_init(char *server, uint16_t port, char *ssl_cert, char *user, char *password, char *topic_prefix, int qos_level, char *client_id);
void mqtt_destroy();
bool mqtt_send(char *topic, char *payload);

#endif
