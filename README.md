# yasdi2mqtt
[![Build](https://github.com/pkwagner/yasdi2mqtt/workflows/build/badge.svg)](https://hub.docker.com/r/pkwagner/yasdi2mqtt)
[![Pulls on Docker Hub](https://img.shields.io/docker/pulls/pkwagner/yasdi2mqtt?color=blue)](https://hub.docker.com/r/pkwagner/yasdi2mqtt)
[![License](https://img.shields.io/github/license/pkwagner/yasdi2mqtt?color=blue)](https://github.com/pkwagner/yasdi2mqtt/blob/master/LICENSE)

`yasdi2mqtt` is a MQTT adapter for SMA inverters communicating using the `yasdi` protocol, allowing for local or IP-based communication. It fetches spot values from attached devices in a fixed interval and publishes them as JSON via MQTT.

[![yasdi2mqtt](.github/assets/mock_console.gif "yasdi2mqtt")](https://hub.docker.com/r/pkwagner/yasdi2mqtt)

<details>
<summary>Full JSON Example</summary>

`yasdi2mqtt` will publish a JSON payload on the MQTT channel `$MQTT_TOPIC_PREFIX/<device_sn>` in the given update interval. Messages will be sent for each inverter individually and have the following format:
```json
{"sn":000,"time":1586608779,"values": {
   "Iac":12580,
   "Uac":239,
   "Fac":50.019998881965876,
   "Pac":3006,
   "Zac":0.17200000816956162,
   "Riso":10000,
   "dI":4,
   "Upv-Ist":416,
   "PPV":3013,
   "E-Total":45358.538154414855,
   "h-Total":47797.772832013434,
   "h-On":51654.766385075491,
   "Netz-Ein":9012,
   "Seriennummer":000,
   "E-Total DC":45694.108978657052,
   "Status":"Mpp",
   "Fehler":"-------"
}}
```
</details>

## Wiring & Hardware
Most of SunnyBoy (not sure for Sunny Island) has an internal connector for `RS485` like this:

<img width="521" alt="image" src="https://github.com/stich86/yasdi2mqtt/assets/27808541/847f35a1-0bf5-472a-9619-d425def81be3">

Usually `D+\D-` are also referred as `A+\B-`, so in case your adapter has `A+\B-` schema, just follow this table:

| PIN | D Schema | A/B Schema |
|-----|----------|------------|
| 2   | D+       | A+         |
| 7   | D-       | B-         |
| 5   | GND      | GND        |

### RS485-over-IP

If you are using [Hi-Flying adapters](http://www.hi-flying.com/) like *EW-11, PW11, PW21 or others*, setup *Serial Port* like this:

<img width="1886" alt="image" src="https://github.com/stich86/yasdi2mqtt/assets/27808541/1603d866-72ad-46bb-95ef-3931e8bdff24">

while *Communication Settings* in this way:

<img width="1890" alt="image" src="https://github.com/stich86/yasdi2mqtt/assets/27808541/b264d945-487d-4d4a-9411-5e3a2fbfea9d">


## Setup & Run
Using the `:alpine` or `:latest` images from [Docker Hub](https://hub.docker.com/r/pkwagner/yasdi2mqtt), you'll get a fresh build every month. Therefore, I'd strongly encourage you choosing a container-based choice from below and taking care of updating the images regularly.

For simplicity, all examples below run `yasdi2mqtt` as container root. For production builds, consider creating an unprivileged user instead.

### Choice I: docker-compose
1. Check `yasdi.ini` configuration
    * The included blueprint is pre-configured for directly attached RS485 adapters. Usually, no work needs to be done here, as you'll define the adapter mapping in step 3.
    * If you want to use IP-based communication instead, you should have a look on the `yasdi` manual or check out [this discussion](https://github.com/pkwagner/yasdi2mqtt/issues/1) (German).
2. Create empty `devices` directory
    * Not necessary, but `yasdi` will use this folder as device data cache, so you'll save 1-2 minutes for device data download after the first startup.
3. Edit `docker-compose.yml` file
    * Check out the table below for the environment variable mapping (section `environment`).
    * You may adjust the serial adapter path (section `devices`, the part before the colon) if not mapped to `/dev/ttyUSB0` or drop it when using an IP-based setup.
4. `docker-compose up`
    * After starting up, `yasdi2mqtt` should immediately connect to your MQTT broker.
    * Detected devices should be printed on `stdout` quite quickly, but the the initial device data download may delay the first data for 1-2 minutes.

### Choice II: docker
<details>
<summary>Docker Setup Instructions</summary>

1. Check `yasdi.ini` configuration
    * The included blueprint is pre-configured for directly attached RS485 adapters. Usually, no work needs to be done here, as you'll define the adapter mapping in step 3.
    * If you want to use IP-based communication instead, you should have a look on the `yasdi` manual or check out [this discussion](https://github.com/pkwagner/yasdi2mqtt/issues/1) (German).
2. Create empty `devices` directory
    * Not necessary, but `yasdi` will use this folder as device data cache, so you'll save 1-2 minutes for device data download after the first startup.
3. Start container with the command below
    * Check out the table below for the environment variable mapping (`-e` flags).
    * You may adjust the serial adapter path (`--device` flag, the part before the colon) if not mapped to `/dev/ttyUSB0` or drop it when using an IP-based setup.
    * After starting up, `yasdi2mqtt` should immediately connect to your MQTT broker.
    * Detected devices should be printed on `stdout` quite quickly, but the the initial device data download may delay the first data for 1-2 minutes.

```sh
docker run \
   --device /dev/ttyUSB0:/dev/ttyUSB0 \
   -v <project_dir>/devices:/etc/yasdi2mqtt/devices \
   -v <project_dir>/yasdi.ini:/etc/yasdi2mqtt/yasdi.ini:ro \
   -e YASDI_CONFIG="/etc/yasdi2mqtt/yasdi.ini" \
   -e YASDI_MAX_DEVICE_COUNT="1" \
   -e YASDI_UPDATE_INTERVAL="30" \
   -e MQTT_TOPIC_PREFIX="/solar/inverter" \
   -e MQTT_SERVER="example.com" \
   -e MQTT_PORT="1883" \
   -e MQTT_USER="johndoe" \
   -e MQTT_PASSWORD="sEcReT" \
   pkwagner/yasdi2mqtt:alpine
```
</details>

### Choice III: Manual setup
<details>
<summary>Manual Setup Instructions (consider using docker-compose or docker instead)</summary>

1. Install [cJSON](https://github.com/DaveGamble/cJSON), [Paho](https://github.com/eclipse/paho.mqtt.c.git), and other project dependencies
   1. `sudo apt install git gcc make cmake openssl libssl-dev libcjson1 libcjson-dev libpaho-mqtt1.3 libpaho-mqtt-dev` (assuming an Ubuntu-based OS)
2. Clone and install [YASDI](https://github.com/konstantinblaesi/yasdi.git)
    1. `mkdir projects/generic-cmake/build-gcc`
    2. `cd projects/generic-cmake/build-gcc`
    3. `cmake -D YASDI_DEBUG_OUTPUT=0 ..`
    4. `make`
    5. `sudo make install`
3. Clone and install this repository
    1. `make YASDI_PATH=<yasdi_dir>`
    2. `sudo make YASDI_PATH=<yasdi_dir> install`
4. Check `yasdi.ini` configuration
    * The included blueprint is pre-configured for directly attached RS485 adapters, but you may want to adjust the serial adapter path.
    * If you want to use IP-based communication instead, you should have a look on the `yasdi` manual or check out [this discussion](https://github.com/pkwagner/yasdi2mqtt/issues/1) (German).
5. Create empty `devices` directory
    * Not necessary, but `yasdi` will use this folder as device data cache, so you'll save 1-2 minutes for device data download after the first startup.
6. Set environment variables according to the table below
7. `./yasdi2mqtt`
    * After starting up, `yasdi2mqtt` should immediately connect to your MQTT broker.
    * Detected devices should be printed on `stdout` quite quickly, but the the initial device data download may delay the first data for 1-2 minutes.
</details>


### Choice IV: Home Assistant Add-On
<details>
<summary>Add this repository as source to your Home Assistant OS</summary>

1. Click here to add this repository to your HA instance
   
   [![Open your Home Assistant instance and show the add add-on repository dialog with a specific repository URL pre-filled.](https://my.home-assistant.io/badges/supervisor_add_addon_repository.svg)](https://my.home-assistant.io/redirect/supervisor_add_addon_repository/?repository_url=https%3A%2F%2Fgithub.com%2Fpkwagner%2Fyasdi2mqtt)
   
2. Go to **Settings** -> **Add-ons** -> **yasdi2mqtt** and on the **Configuration** tab fills all needed information about your MQTT broker like this:
   
<img width="1046" alt="image" src="https://github.com/stich86/yasdi2mqtt/assets/27808541/4ccf697c-9526-46a8-8e2d-4e746c2bcb34">

3. Create `yasdi.ini` file into your `/config` folder using [File editor add-on](https://github.com/home-assistant/addons/blob/master/configurator/README.md) with this content:
    - If you are connecting using ttyUSB adapter you can use this one as sample:
      ```
      [DriverModules]
      Driver0=yasdi_drv_serial

      # Configs for communiation over Ethernet/UDP
      # Replace 192.168.178.9 with the real IP address of your device

      [COM1]
      Device=/dev/ttyUSB0
      Media=RS485
      Baudrate=1200
      Protocol=SMANet
      ```
   - If you are connecting using RS485-over-IP you can use this one as sample:
     ```
      [DriverModules]
      Driver0=yasdi_drv_ip

      # Configs for communiation over Ethernet/UDP
      # Replace 192.168.0.10 with the real IP address of your device, communication is done by UDP ports 24272 and 24273 

      [IP0]
      Protocol=SMANet
      Device0=192.168.0.10
      ```

3. Start the add-on (enable also auto-startup and watch-dog)
    * You should wait about 1-2 minutes before the inverter's data will be visible.

4. Here are some MQTT sensors example that you can add on your Home Assistant instance (based on SunnyBoy SB3000):

   ```
   mqtt:
     sensor:
     - name: "Total Energy (from installation)"
       unique_id: "solar_generated_total"
       device_class: "energy"
       state_class: "total"
       unit_of_measurement: "kWh"
       state_topic: "sma/inverter/20xxxxxx"
       value_template: "{{ value_json['values']['E-Total'] }}"
     - name: "Total run time (from installation)"
       unique_id: "solar_hours_total"
       device_class: "duration"
       state_class: "total"
       unit_of_measurement: "h"
       state_topic: "sma/inverter/20xxxxxx"
       value_template: "{{ value_json['values']['h-Total'] }}"    
     - name: "Instant Power"
       unique_id: "solar_power_watt"
       device_class: "power"
       state_class: "measurement"
       unit_of_measurement: "W"
       state_topic: "sma/inverter/20xxxxxx"
       value_template: "{{ value_json['values']['Pac'] }}"
     - name: "CC Power"
       unique_id: "solar_power_cc"
       device_class: "voltage"
       state_class: "measurement"
       unit_of_measurement: "V"
       state_topic: "sma/inverter/20xxxxxx"
       value_template: "{{ value_json['values']['Upv-Ist'] }}"
     - name: "AC Power"
       unique_id: "solar_power_ac"
       device_class: "voltage"
       state_class: "measurement"
       unit_of_measurement: "V"
       state_topic: "sma/inverter/20xxxxxx"
       value_template: "{{ value_json['values']['Uac'] }}"
     - name: "AC Frequency"
       unique_id: "solar_power_ac_freq"
       device_class: "frequency"
       state_class: "measurement"
       unit_of_measurement: "Hz"
       state_topic: "sma/inverter/20xxxxxx"
       value_template: "{{ value_json['values']['Fac'] }}"
     - name: "Inverter State"
       unique_id: "solar_power_state"
       state_topic: "sma/inverter/20xxxxxx"
       value_template: "{{ value_json['values']['Status'] }}"
     - name: "Inverter Errors"
       unique_id: "solar_power_error"
       state_topic: "sma/inverter/20xxxxxx"
       value_template: "{{ value_json['values']['Fehler'] }}"
    ```

For other data details information, you can refer to your Inverter *Installation\User Guide*, for example SunnyBoy [SB4200](https://files.sma.de/downloads/SB4200-12-FE4804.pdf) has description of each value at paragraph *6.4*

</details>

### Environment variables
| Variable               | Description                                                                                                                                                                   | Example                |
|------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------|
| YASDI_CONFIG           | Path to `yasdi.ini` file <br> *Inside container, shouldn't be changed therefore*                                                                                              | /etc/yasdi2mqtt/yasdi.ini |
| YASDI_MAX_DEVICE_COUNT | Maximum number of devices being online at the same time                                                                                                                       | 1                         |
| YASDI_UPDATE_INTERVAL  | Time between value update requests in seconds <br> *Value update itself takes some time, so it shouldn't be lower than 15 from my experience*                                 | 30                        |
| MQTT_TOPIC_PREFIX      | MQTT messages will later be published to topic `$MQTT_TOPIC_PREFIX/<device_sn>`                                                                                               | solar/inverter            |
| MQTT_SERVER            |                                                                                                                                                                               | example.com               |
| MQTT_PORT              |                                                                                                                                                                               | 1883                      |
| MQTT_SSL_CERT          | *Optional*<br><br>Path to `cert.pem` file<br>*If you want to use MQTTS, you have to define the path to the cert here and add the cert to the container (e.g. mount a volume)* | /etc/certs/cert.pem       |
| MQTT_CLIENT_ID         | *Optional*                                                                                                                                                                    | yasdi2mqtt                |
| MQTT_QOS_LEVEL         | *Optional*<br><br>See [here](http://www.steves-internet-guide.com/understanding-mqtt-qos-levels-part-1/) for explanation.                                                     | 2                         |
| MQTT_USER              | *Optional*                                                                                                                                                                    | johndoe                   |
| MQTT_PASSWORD          | *Optional*                                                                                                                                                                    | sEcReT                    |
| LOG_LEVEL              | *Optional*<br><br>Set `0` to enable debug output                                                                                                                              | 0                         |

## Get Help
Thanks to a growing community of `yasdi2mqtt` users, there is already plenty of information on the [Discussions](https://github.com/pkwagner/yasdi2mqtt/discussions) tab. If you struggle during setup, please check out existing answers first or feel free to open a new thread. Here's a quick overview over some relevant threads:

* [RS485 wiring and adapter recommendations](https://github.com/pkwagner/yasdi2mqtt/discussions/6) (German)
* [IP-based setup](https://github.com/pkwagner/yasdi2mqtt/discussions/5) (German)
* [Integration with Home Assistant](https://github.com/pkwagner/yasdi2mqtt/discussions/18)

### Debugging
There are two options to make `yasdi2mqtt` more verbose:
* Enable the debug output of `yasdi2mqtt` using the `LOG_LEVEL` environment variable from the table above.
* Replace `YASDI_DEBUG_OUTPUT=0` by `YASDI_DEBUG_OUTPUT=1` in the `Dockerfile` to activate the debug output of the underlying `yasdi` library.
    * You'll need to re-compile your own container using `docker build -t yasdi2mqtt .` and change the `docker-compose.yml` file to use your own container.
    * When doing a manual setup, replace the parameter directly when installing `yasdi`.

## License
Unless otherwise stated in the file header, all contributions to this project are licensed under the MIT license. In particular, `src/log.c` and `src/log.h` are NOT part of this project.
