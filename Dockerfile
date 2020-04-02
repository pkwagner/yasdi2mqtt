FROM debian:buster-slim

COPY . /yasdi2mqtt

RUN apt-get update && apt-get install -y git gcc make cmake openssl libssl-dev libcjson1 libcjson-dev \
 && git clone --depth=1 https://github.com/eclipse/paho.mqtt.c.git paho \
 && cd paho && make && make install && cd .. \
 && git clone --depth=1 https://github.com/rxi/log.c.git logc \
 && gcc -shared -DLOG_USE_COLOR -o /usr/local/lib/liblog_c.so logc/src/log.c && cp logc/src/*.h /usr/local/include \
 && git clone --depth=1 https://github.com/konstantinblaesi/yasdi.git yasdi \
 && mkdir yasdi/projects/generic-cmake/build-gcc && cd yasdi/projects/generic-cmake/build-gcc \
 && cmake -D YASDI_DEBUG_OUTPUT=0 .. && make && make install \
 && cd ../../../.. \
 && cd yasdi2mqtt && make YASDI_PATH=../yasdi && make YASDI_PATH=../yasdi install && cd .. \
 && rm -rf paho logc yasdi \
 && apt-get purge -y --auto-remove git gcc make cmake libssl-dev libcjson-dev

# Temporary usage: docker run yasdi2mqtt yasdi2mqtt -c <yasdi_config> -d <yasdi_driver_id> -i <yasdi_max_device_count> -u <yasdi_update_interval> -t <mqtt_topic_prefix> -s <mqtt_server> -p <mqtt_port> (-U <mqtt_user>) (-P <mqtt_password>) (-l <log_level>)
# Friendly reminder to mount yasdi.ini and /dev/ttyUSB0 :)