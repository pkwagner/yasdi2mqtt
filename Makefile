YASDI_PATH = yasdi

build: src/main.c src/yasdi_handler.c src/yasdi_handler.h src/mqtt_client.c src/mqtt_client.h
	gcc -std=c11 -lyasdimaster -llog_c -lcjson -lpaho-mqtt3c -I$(YASDI_PATH)/libs -I$(YASDI_PATH)/include -I$(YASDI_PATH)/core -I$(YASDI_PATH)/driver -I$(YASDI_PATH)/smalib -I$(YASDI_PATH)/master -I$(YASDI_PATH)/os -o yasdi2mqtt src/main.c src/yasdi_handler.c src/mqtt_client.c

install: build
	cp yasdi2mqtt /usr/local/bin