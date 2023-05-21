#include <lib.h>

int pageref(void* v) {
	return ecall_get_pgref(v);
}
