user_dir 	:= ../user
tools_dir	:= ../tools
INCLUDES	:= -I../include -I$(user_dir)/include

include $(user_dir)/include.mk
USERLIB		:= $(addprefix $(user_dir)/,$(USERLIB))
USERAPPS	:= $(addprefix $(user_dir)/,$(USERAPPS))

VIRTIOLIB	:= virtio_disk.o


.PRECIOUS: %.b %.b.c

%.x: %.b.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<
%.b.c: %.b
	$(tools_dir)/bintoc -f $< -o $@ -p virtio
%.b: %.o $(USERLIB) $(VIRTIOLIB)
	$(LD) -o $@ $(LDFLAGS) -T $(user_dir)/user.lds $^
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
%.o: $(user_dir)/lib.h

.PHONY: all clean

all: init.x test1.x $(VIRTIOLIB)

clean: 
	rm -rf *~ *.o *.b.c *.b *.x

