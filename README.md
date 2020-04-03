# yasdi2mqtt
`yasdi2mqtt` is an MQTT adapter for SMA inverters communicating over serial bus. It fetches spot values from attached devices in a fixed interval and publishes them via MQTT in JSON data format. Moreover, it also works with only a fraction of all inverters being online at the same time.

![Grafana setup with yasdi2mqtt](asset/grafana.png "Grafana setup with yasdi2mqtt")

## Setup & Run
There are multiple ways to get `yasdi2mqtt` working. I'd strongly recommend using `docker-compose` or `docker` for setup.

### Approach I: docker-compose
1. Check if `yasdi.ini` file fits your needs
    * Especially the serial adapter mountpoint (`Device` parameter) might differ across systems
2. Edit `docker-compose.yml`
    * Set environmental variables (`environment` section) according to environmental variable reference below
    * Change serial adapter mountpoint in `devices` (if modified in step 1)
3. `$ docker-compose up`
    * Build might take a few minutes
    * After starting up, `yasdi2mqtt` should immediately connect to your MQTT broker
    * Detected devices will be printed to console, nevertheless the first MQTT message might take 1-2 minutes more, since channel list has to be downloaded first
        * Consider setting log level to `DEBUG` if device detection / the first MQTT message takes too much time

### Approach II: docker
> Work in progress. Use docker-compose instead.

### Approach III: Manual setup
> Work in progress. Use docker-compose instead.

### Environmental variables
| Variable               | Description                                                                                                                              | Example value             |
|------------------------|------------------------------------------------------------------------------------------------------------------------------------------|---------------------------|
| YASDI_CONFIG           | Path to `yasdi.ini` file (inside container) <br> *Remind to also modify volume mountpoint when changing*                                  | /etc/yasdi2mqtt/yasdi.ini |
| YASDI_DRIVER_ID        | ID of driver declared in `yasdi.ini` to use                                                                                                | 0                         |
| YASDI_MAX_DEVICE_COUNT | Maximum number of devices being online at the same time                                                                                  | 1                         |
| YASDI_UPDATE_INTERVAL  | Time between value update requests in seconds <br> *Value update itself takes some time, so it shouldn't be lower than 15 from my experience* | 30                        |
| MQTT_TOPIC_PREFIX      | MQTT messages will later be published to topic `$MQTT_TOPIC_PREFIX/<device_sn>`                                                            | solar/inverter            |
| MQTT_SERVER            |                                                                                                                                          | example.com               |
| MQTT_PORT              |                                                                                                                                          | 1883                      |
| MQTT_USER              | Anonymous MQTT sessions are currently not supported (wip)                                                                                | johndoe                   |
| MQTT_PASSWORD          | Anonymous MQTT sessions are currently not supported (wip)                                                                                | sEcReT                    |

## Debugging
> Work in progress.

## TODO
* Allow anonymous MQTT sessions in `docker-entrypoint.sh` (remove parameters if env not set)
* Add optional debug flag to `docker-compose.yml` / `docker-entrypoint.sh`

