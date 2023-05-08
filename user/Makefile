tool_dir	:= ../tools
include include.mk

.PRECIOUS: %.b %.b.c

%.x : %.b.c
	$(CC)  $(CFLAGS) -c -o $@ $<
%.b.c: %.b
   	$(tool_dir)/bintoc -f $< -o $@ -p user
%.b: %.o $(USERLIB)
	$(LD) -o $@ $(LDFLAGS) -T ./user.lds $^
%.o: %.c	
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
$.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: lib.h

.PHONY: all clean

all: $(USERLIB)

clean:
	rm -rf *~ *.o *.b.c *.x *.b lib/*.o



