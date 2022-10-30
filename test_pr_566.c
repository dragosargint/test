#include "test_pr_566.h"

/*
 *	!!! WARNING !!!:
 *	THIS IS A TEST THAT MIXES MUSL CODE WITH LOW LEVEL CODE
 *	BECAUSE THE EASIEST WAY TO TEST SOME LOW LEVEL FUNCTIONS
 *	PROVIDED BY PR #566 IS TO USE MUSL
 */

void test_clone(void);
void test_detached(void);
void test_pthread_basic(void);

int main_thread_tid;
void test_pr_566()
{
	main_thread_tid = syscall(SYS_gettid);

	test_clone();
	test_detached();
	test_pthread_basic();
}
