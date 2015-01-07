CC = gcc
CFLAGS = -g -std=c99 -Wall
LIBS = -lm
OBJS = src/Speedtest.c \
	src/SpeedtestConfig.c \
	src/SpeedtestServers.c \
	src/url.c \
	src/http.c

all:	SpeedTestC

SpeedTestC: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm SpeedTestC

.PHONY: all
