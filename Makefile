YASDI_PATH=yasdi
DESTDIR=/usr/local/bin

IDIRS=-I$(YASDI_PATH)/libs -I$(YASDI_PATH)/include -I$(YASDI_PATH)/core -I$(YASDI_PATH)/driver -I$(YASDI_PATH)/smalib -I$(YASDI_PATH)/master -I$(YASDI_PATH)/os
SRC=src/main.c src/yasdi_handler.c src/mqtt_client.c src/log.c

CC=gcc
CFLAGS=-std=c11 -DLOG_USE_COLOR $(IDIRS)
DEPS=-lyasdimaster -lcjson -lpaho-mqtt3cs

ifeq ($(DEBUG), 1)
	CFLAGS+= -g
endif

yasdi2mqtt: $(SRC)
	$(CC) -o yasdi2mqtt $(SRC) $(CFLAGS) $(DEPS)

install: yasdi2mqtt
	cp yasdi2mqtt $(DESTDIR)
