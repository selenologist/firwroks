CC     := gcc
CFLAGS := -g -Wall -Werror -std=c99

main: main.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm main

install: main
	cp main /sbin/firwroks
	chown 0:0 /sbin/firwroks
	chmod +s /sbin/firwroks
	# !!!

all: main
