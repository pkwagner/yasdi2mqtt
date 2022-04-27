FROM alpine:latest AS build

COPY . /yasdi2mqtt

RUN apk add --no-cache git gcc musl-dev make cmake openssl-dev cjson-dev
RUN ln -s /usr/include/termios.h /usr/include/termio.h

RUN mkdir -p /target/usr/local/lib /target/usr/local/bin

# Install Paho
RUN git clone --depth=1 https://github.com/eclipse/paho.mqtt.c.git paho
RUN mkdir paho/build && cd paho/build && cmake -DPAHO_WITH_SSL=TRUE .. && make && make install && make DESTDIR=/target install

# Install log.c
RUN git clone --depth=1 https://github.com/rxi/log.c.git logc
RUN cd logc && gcc -shared -fPIC -DLOG_USE_COLOR -o liblog_c.so src/log.c
RUN cp logc/src/*.h /usr/local/include && cp logc/src/*.h /target/usr/local/include
RUN cp logc/liblog_c.so /usr/local/lib && cp logc/liblog_c.so /target/usr/local/lib

# Install YASDI
RUN git clone --depth=1 https://github.com/pkwagner/yasdi.git yasdi
RUN mkdir yasdi/projects/generic-cmake/build-gcc
RUN cd yasdi/projects/generic-cmake/build-gcc && cmake -D YASDI_DEBUG_OUTPUT=0 .. && make && make install && make DESTDIR=/target install

# Build yasdi2mqtt
RUN cd yasdi2mqtt && make YASDI_PATH=../yasdi && make YASDI_PATH=../yasdi DESTDIR=/target/usr/local/bin install


FROM alpine:latest

RUN apk add --no-cache cjson

RUN mkdir /etc/yasdi2mqtt
WORKDIR /etc/yasdi2mqtt

COPY --from=build /target/usr/local/lib/*.so* /usr/local/lib/
COPY --from=build /target/usr/local/bin/yasdi2mqtt /usr/local/bin/

ENTRYPOINT ["yasdi2mqtt"]
