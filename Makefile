# For FreshTomato builds, ignored in other cases
-include ../common.mak

OPENSSL_CFLAGS = -DOPENSSL -DURL_PROTOCOL='"https"'
OPENSSL_LIBS = -lssl -lcrypto

CFLAGS = -Os -Wall -std=c99 $(OPENSSL_CFLAGS) $(EXTRACFLAGS)
LDFLAGS	=
LIBS := -lm -lpthread $(OPENSSL_LIBS)

ifdef TOMATO_BUILD
LIBS += -L$(TOP)/openssl
CFLAGS += -I $(TOP)/openssl/include
endif

OBJDIR = .
SRCDIR = src
SOURCES  := $(wildcard $(SRCDIR)/*.c)

OBJS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: SpeedTestC

SpeedTestC: $(OBJS)
	@echo " [SpeedTestC] CC $@"
	@$(CC) -o $@ $(OBJS) $(LIBS)

	$(SIZECHECK)
	$(CPTMP)


install:
	@echo " [SpeedTestC] Installing to $(INSTALLDIR)"
	@install -D SpeedTestC $(INSTALLDIR)/usr/bin/SpeedTestC
	@$(STRIP) $(INSTALLDIR)/usr/bin/SpeedTestC
	@chmod 0500 $(INSTALLDIR)/usr/bin/SpeedTestC

clean:
	rm -f SpeedTestC *.o .*.depend

size: SpeedTestC
	mipsel-uclibc-nm --print-size --size-sort SpeedTestC

$(OBJS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@echo " [SpeedTestC] CC $@"
	@$(CC) $(CFLAGS) -c $< -o $@

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)

test:  SpeedTestC
	valgrind --leak-check=full --show-leak-kinds=all ./SpeedTestC --server http://speedtest.skynet.net.pl/speedtest/upload.php
	valgrind --leak-check=full --show-leak-kinds=all ./SpeedTestC

