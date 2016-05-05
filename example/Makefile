all: hellow.bin pwned spawnshell.bin

hellow.o: hellow.S
	as $^ -o $@

hellow.bin: hellow.o
	objcopy -O binary --only-section=.text $^ $@

spawnshell.o: spawnshell.S
	as $^ -o $@

spawnshell.bin: spawnshell.o
	objcopy -O binary --only-section=.text $^ $@

pwned.o: pwned.S
	as $^ -o $@

pwned.bin: pwned.o
	objcopy -O binary --only-section=.text $^ $@

pwned.h: pwned.bin
	xxd -i $^ $@

pwned: pwned.c pwned.h
	gcc pwned.c -o pwned -Wall -Werror

clean:
	rm *.o *.bin

run-hellow: hellow.bin
	nc -lp 31337 < hellow.bin
	# then kill netcat to force a connection close

