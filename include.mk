
CROSS_COMPILE  := riscv64-unknown-elf-
CC             := $(CROSS_COMPILE)gcc
CFLAGS         += --std=gnu99  -g  -fno-pic -ffreestanding -nostartfiles -fno-stack-protector -fno-builtin -Wall -march=rv32imaf -mabi=ilp32f 
LD             := $(CROSS_COMPILE)ld
LDFLAGS        += -n -nostdlib -m elf32lriscv 
#verbose
GDB 		   := $(CROSS_COMPILE)gdb
GDB_FLAGS	   += --eval-command 'target remote:1234'

HOST_CC 	   := cc
HOST_CFLAGS    := --std=gnu99 -O2 -Wall

