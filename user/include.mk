
USERLIB 				:=  entry.o \
							ecall_wrap.o \
							debugf.o	\
							libos.o		\
							fork.o		\
							ecall_lib.o	\
							ipc.o		\

INITAPPS := fktest.x
USERLIB :=	$(addprefix lib/, $(USERLIB)) $(wildcard ../lib/*.o)
