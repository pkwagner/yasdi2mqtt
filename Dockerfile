FROM debian:bullseye-slim

COPY . /yasdi2mqtt

RUN apt-get update && apt-get install -y git gcc make cmake openssl libssl-dev libcjson1 libcjson-dev libpaho-mqtt1.3 libpaho-mqtt-dev \
 && git clone --depth=1 https://github.com/pkwagner/yasdi.git yasdi \
 && mkdir yasdi/projects/generic-cmake/build-gcc && cd yasdi/projects/generic-cmake/build-gcc \
 && cmake -D YASDI_DEBUG_OUTPUT=0 .. && make && make install \
 && cd ../../../.. \
 && cd yasdi2mqtt && make YASDI_PATH=../yasdi && make YASDI_PATH=../yasdi install && cd .. \
 && rm -rf yasdi \
 && apt-get purge -y --auto-remove git gcc make cmake libssl-dev libcjson-dev libpaho-mqtt-dev && rm -rf /var/lib/apt/lists/* \
 && mkdir /etc/yasdi2mqtt

WORKDIR /etc/yasdi2mqtt

ENTRYPOINT ["yasdi2mqtt"]
