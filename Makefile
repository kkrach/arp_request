#
# Makefile
#
# Call 'make CROSSCOMPILE=' for native compilation
#

TARGET=arp_request
SRCS=main.c

CROSSCOMPILE=/opt/crosstool-ng-powerpc/bin/powerpc-e500v2-linux-gnuspe-

default: $(TARGET)

$(TARGET): $(SRCS)
	$(CROSSCOMPILE)gcc -Wall $(SRCS) -o $(TARGET)

clean:
	rm -rf *.o $(TARGET)

upload: $(TARGET)
	scp -P20160 $(TARGET) root@172.16.10.168:/tmp/

.PHONY: clean upload

