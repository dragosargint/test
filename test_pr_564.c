#include <stdio.h>
#include <uk/sched.h>
#include <uk/print.h>
#include <unistd.h>
#include <uk/list.h>

/*
 * The red lines with "GOOD:" are not errors
 */

#define PRINT_INFO(...)	\
do {	\
	uk_pr_err("GOOD: function %s() called; ", __func__);	\
	struct uk_thread *__t = uk_thread_current();	\
	uk_pr_err("Thread running: %p and name: %s; ", __t, __t->name ? __t->name : "" );	\
	uk_pr_err(__VA_ARGS__);	\
	uk_pr_err("\n");	\
} while(0)

#define PRINT_BAD(...)	\
do {	\
	uk_pr_err("BAD:");	\
	uk_pr_err(__VA_ARGS__);	\
	uk_pr_err("\n");	\
} while(0)

static int FN0 = 0;
static int FN1 = 0;
static int FN2 = 0;
static int DTOR_CALLED = 0;
static int DTOR = 0;

__thread long tls1 = 0xaabbccdd;
__thread long tls2 = 0xaabbccdd;
__thread long tls3 = 0xaabbccdd;

void generic_dtor(struct uk_thread *t)
{
	PRINT_INFO("Destructor will act for uk_thread %p and name %s", t, t->name ? t->name : "");
	DTOR_CALLED += 1;
}

__noreturn void fn0(void)
{
	tls1 = 0xaaaaaaaa;
	PRINT_INFO("0 arguments");
	FN0 = 1;
	DTOR += 1;	
	uk_sched_thread_exit();
}


__noreturn void fn1(void *arg)
{
	tls2 = 0xbbbbbbbb;
	if ((long) arg == 0xdeadbeef)
		PRINT_INFO("1 argument");
	else
		PRINT_BAD("argument for function %s didn't arrive correctly", __func__);
	FN1 = 1;
	DTOR += 1;
	uk_sched_thread_exit();
}

__noreturn void fn2(void *arg1, void *arg2)
{
	tls3 = 0xcccccccc;
	if ((long) arg1 == 0xdeadbeef && (long) arg2 == 0xbadbabee)
		PRINT_INFO("2 arguments");
	else
		PRINT_BAD("the arguments for function %s didn't arrive correctly", __func__);
	
	FN2 = 1;
	DTOR += 1;
	uk_sched_thread_exit();
}


void test_pr_564()
{
	/* take the current scheduler used by the main thread */
	struct uk_sched *scheduler = uk_sched_current();
	struct uk_thread *itr;
	char buf[10000];
	int count = 0;
	int no_threads = 0;

	/* create a thread that will start from fn0, a function with 0 args */
	uk_sched_thread_create_fn0(scheduler,
				   fn0,
				   4096,
				   0,
				   0,
				   "Thread 1",
				   NULL,
				   generic_dtor);

	uk_sched_thread_create_fn1(scheduler,
				   fn1,
				   (void *) 0xdeadbeef,
				   4096,
				   0,
				   0,
				   "Thread 2",
				   NULL,
				   generic_dtor);

	uk_sched_thread_create_fn2(scheduler,
				   fn2,
				   (void *) 0xdeadbeef,
				   (void *) 0xbadbabee,
				   4096,
				   0,
				   0,
				   "Thread 3",
				   NULL,
				   generic_dtor);

	UK_TAILQ_FOREACH(itr, &scheduler->thread_list, thread_list) {	
		count += snprintf(buf + count, sizeof(buf) - count, "\n\t%s at address %p", itr->name, itr);
		no_threads += 1;
	}

	if (no_threads == 5)
		uk_pr_err("GOOD: all 5 threads were found: %s\n", buf);
	else
		PRINT_BAD("%d of 5 threads were found: %s\n", no_threads, buf);

	sleep(2);
	if (!FN0)
		PRINT_BAD("fn0 was not called");
	if (!FN1)
		PRINT_BAD("fn1 was not called");
	if (!FN2)
		PRINT_BAD("fn2 was not called");
	if (DTOR_CALLED < DTOR)
		PRINT_BAD("generic_dtor was not called for all the created threads");
	if (tls1 == 0xaabbccdd && tls2 == 0xaabbccdd && tls3 == 0xaabbccdd) 
		uk_pr_err("GOOD: TLS WORKS!!!\n");
	else
		PRINT_BAD("TLS DOESN'T WORK!!!");

}

