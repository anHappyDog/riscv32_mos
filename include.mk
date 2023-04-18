# ENDIAN is either EL (little endian) or EB (big endian)
ENDIAN         := EL

CROSS_COMPILE  := riscv64-unknown-elf-
CC             := $(CROSS_COMPILE)gcc
CFLAGS         += --std=gnu99  -g  -fno-pic -ffreestanding -nostartfiles -fno-stack-protector -fno-builtin -Wall -march=rv32imafc -mabi=ilp32f 
LD             := $(CROSS_COMPILE)ld
LDFLAGS        += -$(ENDIAN)  -static -n -nostdlib -m elf32lriscv  -verbose

HOST_CC        := cc
HOST_CFLAGS    += --std=gnu99 -O2 -Wall
HOST_ENDIAN    := $(shell lscpu | grep -iq 'little endian' && echo EL || echo EB)

ifneq ($(HOST_ENDIAN), $(ENDIAN))
# CONFIG_REVERSE_ENDIAN is checked in tools/fsformat.c (lab5)
HOST_CFLAGS    += -DCONFIG_REVERSE_ENDIAN
endif
