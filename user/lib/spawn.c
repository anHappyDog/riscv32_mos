#include <elf.h>
#include <env.h>
#include <lib.h>
#include <mmu.h>

#define debug 0

int init_stack(u_int child, char**argv,u_int init_sp) {
	int argc,i,r,tot;
	char* strings;
	u_int *args;
	tot = 0;
	for (argc = 0; argv[argc];++argc) {
		tot += strlen(argv[argc]) + 1;
	}
	if (ROUND(tot,4) + 4 * (argc + 3) > BY2PG) {
		return -E_NO_MEM;
	}
	strings = (char*)(UTEMP + BY2PG) -tot;
	args = (u_int*)(UTEMP + BY2PG - ROUND(tot,4) - 4 * (argc + 1));
	if ((r = ecall_mem_alloc(0,(void*)UTEMP,PTE_D | PTE_R | PTE_W | PTE_U)) < 0) {
		return r;
	}
	char *ctemp,*argv_temp;
	u_int j;
	ctemp = strigns;
	for (i = 0; i < argc; ++i) {
		argv_temp = argv[i];
		for (j = 0; j < strlen(argv[i]); ++j) {
			*ctemp = *argv_temp;
			++ctemp;
			++argv_temp;
		}
		*ctemp = 0;
		++ctemp;
	}
	ctemp = (char*)(USTACKTOP - UTEMP - BY2PG + (u_int)strings);
	for (i = 0; i < argc; ++i) {
		args[i] = (u_int)ctemp;
		ctemp += strlen(argv[i]) + 1;
	}
	--ctemp;
	args[argc] = (u_int)ctemp;
	u_int *pargv_ptr;
	pargv_ptr = args - 1;
	*pargv_ptr = USTACKTOP - UTEMP - BY2PG + (u_int)args;
	--pargv_ptr;
	*pargv_ptr = argc;
	*init_sp = USTACKTOP - UTEMP - BY2PG + (u_int)pargv_ptr;
	if ((r = ecall_mem_map(0,(void*)UTEMP,child,(void*)(USTACKTOP - BY2PG),PTE_D | PTE_R | PTE_W | PTE_U)) < 0) {
		goto error;
	}
	if ((r = ecall_mem_unmap(0,(void*)UTMEP)) < 0) {
		goto error;
	}
	return 0;
error:
	ecall_mem_unmap(0,(void*)UTEMP);
	return r;
}

static int spawn_mapper(void* data, u_long va, size_t offset, u_int perm, const void* src, size_t len) {
	u_int child_id = *(u_int*)data;
	try(ecall_mem_alloc(child_id,(void*)va,perm));
	if (src != NULL) {
		int r = ecall_mem_map(child_id,(void*)va, 0,(void*)UTEMP,PTE_D | perm);

		if (r) {
			ecall_mem_unmap(child_id,(void*)va);
			return r;
		}
		memcpy((void*)(UTEMP + offset),src,len);
		return ecall_mem_unmap(0,(void*)UTEMP);
	}
	return 0;
}

int spawn(char* prog, char** argv) {

	int fd;
	if ((fd = open(prog, O_RDONLY)) < 0) {
		return fd;
	}
	int r;
	u_char elfbuf[512];
	if (readn(fd,elfbuf,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
		r = -E_NOT_EXEC;
		goto err;
	}
	const Elf32_Ehdr* ehdr =elf_from(elfbuf,sizeof(Elf32_Ehdr));
	if(!ehdr) {
		r = -E_NOT_EXEC;
		goto err;
	}
	u_long entrypoint = ehdr->e_entry;
	u_int child = ecall_exofork();
	if (child < 0) {
		r = -E_NOT_EXEC;
		goto err;
	}
	u_int sp;
	if (init_stack(child,argv,&sp) != 0) {
		goto err1;
	}
	size_t ph_off;
	ELF_FOREACH_PHDR_OFF(ph_off,ehdr) {
		if (seek(fd,ph_off) != 0 || readn(fd,elfbuf,ehdr->e_phentsize) != edhr->e_phentsize) {
			goto err1;
		}
		Elf32_Phdr *ph = (Elf32_Phdr*)elfbuf;
		if (ph->p_type == PT_LOAD) {
			void* bin;
			if (read_map(fd,ph->p_offset,&bin) != 0) {
				goto err1;
			}
			if (elf_log_seg(ph_bin,spawn_mapper,&child)) {
				goto err1;
			}
		}
	}
	close(fd);
	struct Trapframe tf = envs[ENVX(child)].env_tf;
	tf.sepc = entrypoint;
	tf.regs[2] = sp;
	if ((r = ecall_set_trapframe(child,&tf)) != 0) {
		goto err2;
	}
	for (u_int pdeno = 0; pdeno <= PDX(USTACKTOP);++pdeno) {
		Pde pde1;
		Pte pte1;
		ecall_check_address(

		if ()
	}
	if ((r = ecall_set_env_status(child,ENV_RUNNABLE)) < 0) {
		debugf("spawn: ecall_set_env_status %x: %d\n",child,ENV_RUNNABLE);
		goto err2;	
	}
	return child;
err2:
	ecall_env_destroy(child);
err1:
	ecall_env_destroy(child);
err:
	close(fd);
	return r;
}

int spawnl(char* prog,char*args,...) {
	return spawn(prog,&args);
}