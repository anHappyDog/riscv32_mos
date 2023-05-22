
USERLIB 				:=  entry.o \
							ecall_wrap.o \
							debugf.o	\
							libos.o		\
							fork.o		\
							ecall_lib.o	\
							ipc.o		\
							pageref.o	\
							console.o	\
							fsipc.o		\
							fd.o		\
							file.o		\
							fprintf.o	

INITAPPS := fktest.x ppa.x test1.x test2.x testipc1.x testipc2.x
USERLIB :=	$(addprefix lib/, $(USERLIB)) $(wildcard ../lib/*.o)
