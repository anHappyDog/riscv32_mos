include include.mk

target_dir              := target
mos_elf                 := $(target_dir)/mos
link_script             := kernel.lds


user_modules			:= user/bare
modules                 := lib init kern
targets                 := $(mos_elf)

qemu32_gdb_flags 		+= -S -s
qemu32_files            += $(mos_elf)
qemu32 					+= qemu-system-riscv32
qemu32_flags            += -m 64M -machine virt -nographic

objects                 := $(addsuffix /*.o, $(modules)) $(addsuffix /*.x, $(user_modules)) 
modules                 += $(user_modules)


.PHONY: all test tools $(modules) clean run gdb objdump  clean-and-all

.ONESHELL:
clean-and-all: clean
	$(MAKE) all


include mk/tests.mk mk/profiles.mk
export CC CFLAGS LD LDFLAGS

all: $(targets)

$(target_dir):
	mkdir -p $@

tools:
	CC="$(HOST_CC)" CFLAGS="$(HOST_CFLAGS)" $(MAKE) --directory=$@

$(modules): tools
	$(MAKE) --directory=$@

$(mos_elf): $(modules) $(target_dir)
	$(LD) $(LDFLAGS) -o $(mos_elf) -N -T $(link_script) $(objects)


clean:
	for d in * tools/readelf user/* tests/*; do
		if [ -f $$d/Makefile ]; then
			$(MAKE) --directory=$$d clean
		fi
	done
	rm -rf *.o *~ $(target_dir) include/generated
	find . -name '*.objdump' -exec rm {} ';'

run:
	$(qemu32) $(qemu32_flags)  -kernel $(qemu32_files)
gdb:
	$(qemu32) $(qemu32_flags) $(qemu32_gdb_flags)  -kernel $(qemu32_files)
gdbstart:
	$(GDB) $(GDB_FLAGS) $(qemu32_files)

objdump:
	@find * \( -name '*.b' -o -path $(mos_elf) \) -exec sh -c \
	'$(CROSS_COMPILE)objdump {} -aldS > {}.objdump && echo {}.objdump' ';'
