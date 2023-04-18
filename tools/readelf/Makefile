%.o: %.c
	$(CC) -c -g $<

.PHONY: clean

readelf: main.o readelf.o
	$(CC) $^ -g -o $@

hello: hello.c
	$(CC) $^ -o $@ -m32 -static -g

clean:
	rm -f *.o readelf hello
