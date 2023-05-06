#include <env.h>
#include <pmap.h>
#include <printk.h>

void schedule(int yield) {
	static int count = 0;
	struct Env* e = curenv;
	--count;
	if (e == NULL||((e != NULL) && (e->env_status != ENV_RUNNABLE))) {
		e = TAILQ_FIRST(&env_sched_list);
		count = e->env_pri;		
	}
	else if (yield != 0 || count <= 0) {
		if (e->env_status == ENV_RUNNABLE) {
			TAILQ_REMOVE(&env_sched_list,e,env_sched_link);
			TAILQ_INSERT_TAIL(&env_sched_list,e,env_sched_link);
		}
		e = TAILQ_FIRST(&env_sched_list);
		count = e->env_pri;
	}
	if (e == NULL) {
		panic("schedule : e is null !\n");
	}
	env_run(e);
}
