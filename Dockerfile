FROM debian:buster-slim

COPY . /yasdi2mqtt

RUN apt-get update && apt-get install -y git gcc make cmake openssl libssl-dev libcjson1 libcjson-dev \
 && git clone --depth=1 https://github.com/eclipse/paho.mqtt.c.git paho \
 && cd paho && make && make install && cd .. \
 && git clone --depth=1 https://github.com/rxi/log.c.git logc \
 && gcc -shared -fPIC -DLOG_USE_COLOR -o /usr/local/lib/liblog_c.so logc/src/log.c && cp logc/src/*.h /usr/local/include \
 && git clone --depth=1 https://github.com/pkwagner/yasdi.git yasdi \
 && mkdir yasdi/projects/generic-cmake/build-gcc && cd yasdi/projects/generic-cmake/build-gcc \
 && cmake -D YASDI_DEBUG_OUTPUT=0 .. && make && make install \
 && cd ../../../.. \
 && cd yasdi2mqtt && make YASDI_PATH=../yasdi && make YASDI_PATH=../yasdi install && cd .. \
 && rm -rf paho logc yasdi \
 && apt-get purge -y --auto-remove git gcc make cmake libssl-dev libcjson-dev && rm -rf /var/lib/apt/lists/* \
 && mkdir /etc/yasdi2mqtt

WORKDIR /etc/yasdi2mqtt

ENTRYPOINT ["yasdi2mqtt"]
