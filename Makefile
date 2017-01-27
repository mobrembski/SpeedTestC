CC = gcc
CFLAGS = -O0 -g -std=c99 -Wall
LIBS = -lm -lpthread
OBJS = src/Speedtest.c \
	src/SpeedtestConfig.c \
	src/SpeedtestServers.c \
	src/SpeedtestLatencyTest.c \
	src/SpeedtestDownloadTest.c \
	src/SpeedtestUploadTest.c \
	src/url.c \
	src/http.c

all:	SpeedTestC

SpeedTestC: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test:	SpeedTestC
	valgrind --leak-check=full --show-leak-kinds=all ./SpeedTestC --server http://speedtest.skynet.net.pl/speedtest/upload.php
	valgrind --leak-check=full --show-leak-kinds=all ./SpeedTestC

clean:
	rm SpeedTestC

.PHONY: all
