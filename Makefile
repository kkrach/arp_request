#
# Makefile
#
# Call 'make CROSSCOMPILE=' for native compilation
#

TARGET=arp_request
SRCS=main.c
CFLAGS=-Wall -Wextra -Werror -std=c99 -D_BSD_SOURCE

CROSSCOMPILE=/opt/crosstool-ng-powerpc/bin/powerpc-e500v2-linux-gnuspe-

default: all
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CROSSCOMPILE)gcc $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -rf *.o $(TARGET)

upload: $(TARGET)
	scp -P20160 $(TARGET) root@172.16.10.168:/tmp/

.PHONY: clean upload

