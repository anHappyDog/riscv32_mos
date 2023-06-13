tools_dir	:= ../tools
INCLUDES 	:= -I../include -Iinclude
include include.mk

.PRECIOUS: %.b %.b.c

%.x : %.b.c
	$(CC)  $(CFLAGS) -c -o $@ $<

%.b.c: %.b
	$(tools_dir)/bintoc -f $< -o $@ -p user

%.b: %.o $(USERLIB)
	$(LD) -o $@ $(LDFLAGS) -T ./user.lds $^

%.o: %.c	
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: lib.h

.PHONY: all clean

all: $(USERLIB) $(INITAPPS) $(USERAPPS)

clean:
	rm -rf *~ *.o *.b.c *.x *.b lib/*.o



