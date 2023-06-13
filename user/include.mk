
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
							fprintf.o	\
							wait.o		\
							spawn.o		\
							pipe.o		
USERAPPS				:=  num.b		\
							echo.b		\
							halt.b		\
							ls.b		\
							sh.b		\
							cat.b		\
							init.b 		\
							test1.b

INITAPPS := icode.x test1.x 
USERLIB :=	$(addprefix lib/, $(USERLIB)) $(wildcard ../lib/*.o)
