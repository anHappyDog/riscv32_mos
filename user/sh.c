#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

int _gettoken(char*s, char** p1, char**p2) {
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}
	while(strchr(WHITESPACE,*s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;	
	}
	if (strchr(SYMBOLS,*s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}
	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS,*s)) {
		++s;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char* s, char**p1) {
	static int c,nc;
	static char* np1,*np2;
	if (s) {
		nc = _gettoken(s,&np1,&np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2,&np1,&np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char** argv, int*rightpipe) {
	int argc = 0;
	while(1) {
		char* t;
		int fd;
		int c = gettoken(0,&t);
		switch (c) {
			case 0:
				return argc;
			case 'w':
				if (argc >= MAXARGS) {
					debugf("too many arguments\n");
					exit();
				}
				argv[argc++] = t;
				break;
			case '<':
				if (gettoken(0,&t) != 'w') {
					debugf("syntax error : < not followed by word\n");
					exit();
				}
				fd = open(t,O_RDONLY);
				if (fd < 0) {
					fd = open(t,O_RDONLY| O_CREAT);
					if (fd < 0) {
						user_panic("file can't be opened!\n");
					}
				}
				dup(fd,0);
				close(fd);
				break;
			case '>':
				if (gettoken(0,&t) != 'w') {
					debugf("syntax error : < not followed by word\n");
					exit();
				}
				fd = open(t,O_WRONLY);
				if (fd < 0) {
					fd = open(t,O_WRONLY | O_CREAT);
					if (fd < 0) {
						user_panic("file can't be opened!\n");
					}
				}
				dup(fd,1);
				close(fd);
				break;
			case '|':
				int p[2];
				pipe(p);
				if ((*rightpipe = fork()) == 0) {
					dup(p[0],0);
			//	debugf("*rightpipe is %08x,1 ref is %08x,0 ref is %08x\n",*rightpipe,pageref(num2fd(1)),pageref(num2fd(0)));
					close(p[0]);
					close(p[1]);
				//	debugf("fd1 ref is %08x,fd0 ref is %08x, pipe ref is %08x\n",pageref(num2fd(p[1])),pageref(num2fd(p[0])),pageref(fd2data(num2fd(p[1]))));
					return parsecmd(argv,rightpipe);
				} else {
					dup(p[1],1);
			//	debugf("*rightpipe is %08x,1 ref is %08x,0 ref is %08x\n",*rightpipe,pageref(num2fd(1)),pageref(num2fd(0)));
					close(p[1]);
					close(p[0]);
				//	debugf("fd1 ref is %08x,fd0 ref is %08x, pipe ref is %08x\n",pageref(num2fd(p[1])),pageref(num2fd(p[0])),pageref(fd2data(num2fd(p[1]))));
					return argc;
				}
				break;
		} 

	}
	return argc;
}

int runcmd(char* s) {
	gettoken(s,0);
	char* argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv,&rightpipe);
	if (argc == 0) {
		return 1;
	}
	argv[argc] = 0;
	int child = spawn(argv[0],argv);
	close_all();
	if (child >= 0) {
		wait(child);
	} else {
		debugf("spwan %s: %d\n",argv[0],child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

/*
static int ReadAcharForCmd(char* ch) {
	char t;
	int r;
	if ((r = read(0,&t,1)) != 1) {
		return r;
	}
	if (t == 27) {
		if ((r = read(0,&t,1)) != 1) {	
			return r;
		}
		if (t == 91) {
			if ((r = read(0,&t,1)) != 1) {	
			return r;
		}
			if (t == CURSOR_RIGHT) {
				t = CURSOR_FORMER_RIGHT;
			} else if (t == CURSOR_LEFT) {
				t = CURSOR_FORMER_LEFT;
			} else if (t == CURSOR_UP) {
				t = CURSOR_FORMER_UP;
			} else if (t == CURSOR_DOWN) {
				t = CURSOR_FORMER_DOWN;
			}
		}
	} 
	*ch = t;
	return r;
}
*/


static void moveCursor(int c) {
	printf("%c%c%c",27,91,c);
}

static void deleteCharFromCmd(char*buf, int*curIndex, int*allIndex) {
	if (*curIndex > 0) {
		for (int i = *curIndex -1; i < *allIndex -1; ++i) {
			buf[i] = buf[i + 1];
		}
		*allIndex -= 1;
		*curIndex -= 1;
		printf("\b");
		write(0,buf + *curIndex,*allIndex - *curIndex);
		printf(" ");
		for (int i = *curIndex;i < *allIndex + 1; ++i) {
			printf("\b");
		}
	}
}

static void AddCharToCmd(char* buf, char ch, int*curIndex,int* allIndex) {
	for (int i = *allIndex; i > *curIndex; --i) {
		buf[i] = buf[i - 1];
	}
	buf[*curIndex] = ch;
	*curIndex += 1;
	*allIndex += 1;
	write(0,buf + *curIndex,*allIndex - *curIndex);
	for (int i = *curIndex; i < *allIndex; ++i) {
		printf("\b");
	}
}


static void doWithDirection(signed char* buf,int *curCmdCnt,int* curIndex,int*allIndex) {
	
	switch(*buf) {
		case CURSOR_FORMER_RIGHT:
			if (*curIndex < *allIndex) {
				moveCursor(CURSOR_RIGHT);
				*curIndex += 1;
			}
			break;
		case CURSOR_FORMER_LEFT:
			if (*curIndex > 0) {
				moveCursor(CURSOR_LEFT);
				*curIndex -= 1;
			}
			break;
		default:
		break;	
	}

}

void readline(char* buf, u_int n) {
	int r,curIndex = 0,allIndex = 0,curCmdCnt = 0;
	signed char ch;
	for (allIndex = 0; allIndex < n;) {
		if ((r = read(0,&ch,1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n",r);
			}
			exit();
		}
		if (ch < 0) {
			doWithDirection(&ch,&curCmdCnt,&curIndex,&allIndex);
		} else {
			if (ch == '\b' || ch == 0x7f) {
				deleteCharFromCmd(buf,&curIndex,&allIndex);
			} else if (ch == '\r' || ch == '\n') {
				buf[allIndex] = 0;
				return;
			} else {
				AddCharToCmd(buf,ch,&curIndex,&allIndex);
			}

		}
	}	
	/*for (int i = 0; i < n;++i) {
		if ((r = read(0,buf + i,1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n",r);
			}
			exit();
		}
		if (buf[i] == '\b' || buf[i] == 0x7f) {
			if (i > 0) {
				i -= 2;
			} else {
				i = -1;
			}
			if (buf[i] != '\b') {
				printf("\b");
			}
		}
		if (buf[i] == '\r' || buf[i] == '\n') {
			buf[i] = 0;
			return;
		}

	}*/
	debugf("line too long\n");
	while ((r = read(0,buf,1)) == 1 && buf[0] != '\r' && buf[0] != '\n');
	buf[0] = 0;
}
char buf[1024];

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

int main(int argc, char**argv) {
	int r;
	int interactive = iscons(0);
	int echocmods = 0;
	debugf("\n::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                  ::\n");
	debugf("::                MOS Shell 2023                    ::\n");
	debugf("::                                                  ::\n");
	debugf("::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN {
		case 'i':
			interactive = 1;
			break;
		case 'x':
			echocmods = 1;
			break;
		default:
			usage();
	}
	ARGEND

		if (argc > 1) {
			usage();
		}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1],O_RDONLY)) < 0) {
			user_panic("open %s: %d",argv[1],r);
		}
		user_assert(r == 0);
	}
	
	for (;;) {
	
		if (interactive) {
			printf("\n$ ");
		}
		readline(buf,sizeof(buf));
		if (buf[0] == '#') {
			continue;
		} 
		if (echocmods) {
			printf("# %s\n",buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork : %d\n",r);
		}
		if (r == 0) {
			runcmd(buf);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}


