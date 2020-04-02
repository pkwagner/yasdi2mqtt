# yasdi2mqtt
`yasdi2mqtt` is an MQTT adapter for SMA inverters communicating over serial bus. It fetches spot values from attached devices in a fixed interval and publishes them via MQTT in JSON data format. Moreover, it also works with only a fraction of all inverters being online, so that it's perfectly compatible with solar inverters being offline during nighttime.

## Setup & Run
There are multiple ways to get `yasdi2mqtt` working. I'd strongly recommend using `docker-compose` or `docker` for setup.

### Approach I: docker-compose
1. Check if `yasdi.ini` file fits your need
    * Especially the serial adapter mountpoint (`Device` parameter) might differ across systems
2. Edit `docker-compose.yml`
    * Set environmental variables (`environment` section) according to environmental variable reference below
    * Change serial adapter in `devices`, if modified in step 1
3. `docker-compose up`
    * Build might take a few minutes
    * After starting up, `yasdi2mqtt` should immediately connect to your MQTT broker
    * Detected devices will be printed to console, nevertheless the first MQTT messages might take 1-2 minutes more, since channel list has to be downloaded first
        * Consider setting log level to `DEBUG` when device detection / the first MQTT message takes too long

### Approach II: docker
> Work in progress. Use docker-compose instead.

### Approach III: Manual setup
> Work in progress. Use docker-compose instead.

### Environmental variables
> Work in progress. See docker-compose.yml.

## Debugging
> Work in progress.

## TODO
* Allow anonymous MQTT sessions in `docker-entrypoint.sh` (remove parameters if env not set)
* Add optional debug flag to `docker-compose.yml` / `docker-entrypoint.sh`

