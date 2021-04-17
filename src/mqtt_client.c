#include "mqtt_client.h"

#include <unistd.h>
#include <string.h>
#include <log.h>
#include <MQTTClient.h>

#define CLIENT_ID "yasdi2mqtt"
#define MQTT_KEEP_ALIVE_INTERVAL 20
#define MQTT_DISCONNECT_TIMEOUT 1000
#define MQTT_RECONNECT_INTERVAL 10
#define MQTT_SSL_VERSION MQTT_SSL_VERSION_TLS_1_2
#define MAX_SERVER_URL_SIZE 128
#define MAX_TOPIC_NAME_SIZE 128

char *topic_pfx;
int qos_lvl;
MQTTClient client;
MQTTClient_connectOptions options;
MQTTClient_SSLOptions ssl_options;

bool mqtt_connect();
void mqtt_conn_lost_cb(void *context, char *cause);
int mqtt_msg_arrived_cb(void *context, char *topicName, int topicLen, MQTTClient_message *message);

bool mqtt_init(char *server, uint16_t port, char *ssl_cert, char *user, char *password, char *topic_prefix, int qos_level)
{
    int status;
    topic_pfx = topic_prefix;
    qos_lvl = qos_level;

    options = (MQTTClient_connectOptions)MQTTClient_connectOptions_initializer;
    options.keepAliveInterval = MQTT_KEEP_ALIVE_INTERVAL;
    options.cleansession = false;

    if (user != NULL && password != NULL)
    {
        options.username = user;
        options.password = password;
    }

    if (ssl_cert != NULL)
    {
        log_debug("Setting up ssl for mqtt connection");

        ssl_options = (MQTTClient_SSLOptions)MQTTClient_SSLOptions_initializer;
        ssl_options.trustStore = ssl_cert;
        ssl_options.sslVersion = MQTT_SSL_VERSION;

        options.ssl = &ssl_options;
    }

    char url[MAX_SERVER_URL_SIZE];
    if (strlen(server) >= MAX_SERVER_URL_SIZE - 12)
    {
        log_fatal("MQTT server exceeds maximum length");
        return false;
    }
    sprintf(url, "%s://%s:%u", options.ssl ? "ssl" : "tcp", server, port);

    status = MQTTClient_create(&client, url, CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (status != MQTTCLIENT_SUCCESS)
    {
        log_fatal("Error while creating MQTTClient instance: %d", status);
        return false;
    }

    status = MQTTClient_setCallbacks(client, NULL, mqtt_conn_lost_cb, mqtt_msg_arrived_cb, NULL);
    if (status != MQTTCLIENT_SUCCESS)
    {
        log_fatal("Error while setting MQTTClient callbacks: %d", status);
        return false;
    }

    return mqtt_connect();
}

bool mqtt_connect()
{
    int status;

    do
    {
        status = MQTTClient_connect(client, &options);

        switch (status)
        {
        case MQTTCLIENT_SUCCESS:
            log_info("Connection to mqtt broker established");
            return true;
        case 1:
            log_warn("Connection refused while connecting to mqtt broker: Unacceptable protocol version");
            return false;
        case 2:
            log_warn("Connection refused while connecting to mqtt broker: Identifier rejected");
            return false;
        case 3:
            // Continue connection attempts
            log_warn("Connection refused while connecting to mqtt broker: Server unavailable");
            break;
        case 4:
            log_warn("Connection refused while connecting to mqtt broker: Bad user name or password");
            return false;
        case 5:
            log_warn("Connection refused while connecting to mqtt broker: Not authorized");
            return false;
        default:
            log_warn("Unknown error while connecting to mqtt broker (status: %d). Going to retry in %d seconds...", status, MQTT_RECONNECT_INTERVAL);
        }

        sleep(MQTT_RECONNECT_INTERVAL);
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
    msg.qos = qos_lvl;
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
    // Empty callback
}
