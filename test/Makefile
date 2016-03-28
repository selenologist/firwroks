all: hellow.bin

hellow.o: hellow.S
	as $^ -o $@

hellow.bin: hellow.o
	objcopy -O binary --only-section=.text $^ $@

clean:
	rm *.o *.bin

run-hellow: hellow.bin
	nc -lp 31337 < hellow.bin
	# then kill netcat to force a connection close

