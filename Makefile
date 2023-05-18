include include.mk

lab						:= 4

target_dir              := target
mos_elf                 := $(target_dir)/mos
link_script             := kernel.lds
user_disk				:= $(target_dir)/fs.img
dtb_file				:= $(target_dir)/virt.dtb
dts_file				:= $(target_dir)/virt.dts

user_modules			:= user user/bare
modules                 := lib init kern
targets                 := $(mos_elf) fs-image 

qemu32_gdb_flags 		+= -S -s
qemu32_files            += $(mos_elf)
qemu32 					+= qemu-system-riscv32
qemu32_flags            += -m 64M -nographic -machine virt
disk_flags				:= -global virtio-mmio.force-legacy=false -drive file=$(user_disk),if=none,format=raw,id=hd -device virtio-blk-device,drive=hd

objects                 := $(addsuffix /*.o, $(modules)) $(addsuffix /*.x, $(user_modules)) 
modules                 += $(user_modules)


.PHONY: all test tools dts dtb fs-image  $(modules) clean run gdb objdump  clean-and-all

.ONESHELL:
clean-and-all: clean
	$(MAKE) all

test: export test_dir = tests/lab$(lab)
test: clean-and-all


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

fs-image: $(target_dir) user
	$(MAKE) --directory=fs image fs-files="$(addprefix ../, $(fs-files))"

fs: user
user: lib


clean:
	for d in * tools/readelf user/* tests/*; do
		if [ -f $$d/Makefile ]; then
			$(MAKE) --directory=$$d clean
		fi
	done
	rm -rf *.o *~ $(target_dir) include/generated
	find . -name '*.objdump' -exec rm {} ';'

run:
	$(qemu32) $(qemu32_flags)  -kernel $(qemu32_files) $(disk_flags)
dts: dtb
	dtc -I dtb -O dts $(dtb_file) > $(dts_file)

dtb:$(mos_elf)
	$(qemu32) $(qemu32_flags),dumpdtb=$(dtb_file) -kernel $(qemu32_files)
gdb:
	$(qemu32) $(qemu32_flags) $(qemu32_gdb_flags)  -kernel $(qemu32_files) $(disk_flags)
gdbstart:
	$(GDB) $(GDB_FLAGS) $(qemu32_files)

objdump:
	@find * \( -name '*.b' -o -path $(mos_elf) \) -exec sh -c \
	'$(CROSS_COMPILE)objdump {} -aldS > {}.objdump && echo {}.objdump' ';'
