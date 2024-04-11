.PHONY: all test $(SUBDIRS)
SOURCES=main.c

all : main.c
	$(CC) -Wall -DTEST_TIME $(SOURCES) -o xor_crypto

clean :
	rm -f $(SOURCES) xor_crypto
