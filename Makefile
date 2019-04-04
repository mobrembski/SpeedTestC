# comment out to build w/o OPENSSL support (won't work w/ www.speedtest.net)
OPENSSL_CFLAGS = -DOPENSSL -DURL_PROTOCOL='"https"'
OPENSSL_LIBS = -lssl -lcrypto

CC = cc
CFLAGS = -O3 -g -std=c99 -Wall $(OPENSSL_CFLAGS)
LIBS = -lm -lpthread $(OPENSSL_LIBS)

SRCS = src/Speedtest.c \
	src/SpeedtestConfig.c \
	src/SpeedtestServers.c \
	src/SpeedtestLatencyTest.c \
	src/SpeedtestDownloadTest.c \
	src/SpeedtestUploadTest.c \
	src/url.c \
	src/http.c

all:	SpeedTestC

SpeedTestC: $(SRCS)
	$(CC) -o $@ $(SRCS) $(CFLAGS) $(LIBS)

test:	SpeedTestC
	valgrind --leak-check=full --show-leak-kinds=all ./SpeedTestC --server http://speedtest.skynet.net.pl/speedtest/upload.php
	valgrind --leak-check=full --show-leak-kinds=all ./SpeedTestC

clean:
	rm SpeedTestC

.PHONY: all
