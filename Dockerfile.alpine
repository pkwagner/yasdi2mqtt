FROM alpine:latest AS build

COPY . /yasdi2mqtt

RUN apk add --no-cache git gcc musl-dev make cmake openssl-dev cjson-dev
RUN ln -s termios.h /usr/include/termio.h

# Prepare DESTDIR for log.c and yasdi2mqtt
RUN mkdir -p /target/usr/local/lib /target/usr/local/bin

# Install Paho
RUN git clone --depth=1 https://github.com/eclipse/paho.mqtt.c.git paho
RUN mkdir paho/build && cd paho/build && cmake -DPAHO_WITH_SSL=TRUE .. && make && make DESTDIR=/target install

# Install log.c
RUN git clone --depth=1 https://github.com/rxi/log.c.git logc
RUN cd logc && gcc -shared -fPIC -DLOG_USE_COLOR -o /target/usr/local/lib/liblog_c.so src/log.c && cp src/*.h /target/usr/local/include

# Install YASDI
RUN git clone --depth=1 https://github.com/pkwagner/yasdi.git yasdi
RUN mkdir yasdi/projects/generic-cmake/build
RUN cd yasdi/projects/generic-cmake/build && cmake -D YASDI_DEBUG_OUTPUT=0 .. && make && make DESTDIR=/target install

# Build yasdi2mqtt
ENV LIBRARY_PATH="/target/usr/local/lib:${LIBRARY_PATH}" LD_LIBRARY_PATH="/target/usr/local/lib:${LD_LIBRARY_PATH}" C_INCLUDE_PATH="/target/usr/local/include:${C_INCLUDE_PATH}"
RUN cd yasdi2mqtt && make YASDI_PATH=../yasdi && make YASDI_PATH=../yasdi DESTDIR=/target/usr/local/bin install


FROM alpine:latest

RUN apk add --no-cache cjson

RUN mkdir /etc/yasdi2mqtt
WORKDIR /etc/yasdi2mqtt

COPY --from=build /target/usr/local/lib/*.so* /usr/local/lib/
COPY --from=build /target/usr/local/bin/yasdi2mqtt /usr/local/bin/

ENTRYPOINT ["yasdi2mqtt"]
