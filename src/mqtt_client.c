#include "mqtt_client.h"

#include <unistd.h>
#include <string.h>
#include <log.h>
#include <MQTTClient.h>

#define CLIENT_ID "yasdi2mqtt"
#define MQTT_KEEP_ALIVE_INTERVAL 20
#define MQTT_DISCONNECT_TIMEOUT 1000
#define MAX_SERVER_URL_SIZE 128
#define MAX_TOPIC_NAME_SIZE 128

char *topic_pfx;
MQTTClient client;
MQTTClient_connectOptions options;

bool mqtt_connect();
void mqtt_conn_lost_cb(void *context, char *cause);
int mqtt_msg_arrived_cb(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void mqtt_msg_delivered_cb(void *context, MQTTClient_deliveryToken dt);

bool mqtt_init(char *server, uint16_t port, char *user, char *password, char *topic_prefix)
{
    topic_pfx = topic_prefix;

    options = (MQTTClient_connectOptions)MQTTClient_connectOptions_initializer;
    if (user != NULL && password != NULL)
    {
        options.username = user;
        options.password = password;
    }
    options.keepAliveInterval = MQTT_KEEP_ALIVE_INTERVAL;
    options.cleansession = false;

    char url[MAX_SERVER_URL_SIZE];
    if (strlen(server) >= MAX_SERVER_URL_SIZE - 12)
    {
        log_fatal("MQTT server exceeds maximum length");
        return false;
    }
    sprintf(url, "tcp://%s:%u", server, port);

    MQTTClient_create(&client, url, CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(client, NULL, mqtt_conn_lost_cb, mqtt_msg_arrived_cb, mqtt_msg_delivered_cb);

    return mqtt_connect();
}

bool mqtt_connect()
{
    int status;

    do
    {
        status = MQTTClient_connect(client, &options);

        // TODO Handle concrete errors
        switch (status)
        {
        case MQTTCLIENT_SUCCESS:
            log_info("Connection to mqtt broker established");
            return true;
        default:
            log_warn("Unknown error while connecting to mqtt broker (status: %d). Going to retry in 10 seconds...", status);
        }

        sleep(10);
    } while (status != MQTTCLIENT_SUCCESS);
}

void mqtt_destroy()
{
    MQTTClient_disconnect(client, MQTT_DISCONNECT_TIMEOUT);
    mqtt_destroy(&client);
}

bool mqtt_send(char *topic, char *payload)
{
    char topic_with_pfx[MAX_TOPIC_NAME_SIZE];
    if (strlen(topic_pfx) + strlen(topic) >= MAX_TOPIC_NAME_SIZE)
    {
        log_error("Topic name exceeds maximum length");
        return false;
    }
    sprintf(topic_with_pfx, "%s/%s", topic_pfx, topic);

    MQTTClient_message msg = MQTTClient_message_initializer;
    msg.payload = payload;
    msg.payloadlen = strlen(payload);
    msg.qos = 2;
    msg.retained = false;

    int status = MQTTClient_publishMessage(client, topic_with_pfx, &msg, NULL);
    if (status != MQTTCLIENT_SUCCESS)
    {
        log_error("Unable to publish via mqtt (status: %d)", status);
        return false;
    }

    return true;
}

void mqtt_conn_lost_cb(void *context, char *cause)
{
    log_warn("Lost connection to mqtt broker: %s", cause);
    mqtt_connect();
}

int mqtt_msg_arrived_cb(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    // TODO
}

void mqtt_msg_delivered_cb(void *context, MQTTClient_deliveryToken dt)
{
    // TODO
}