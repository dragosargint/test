#include "test_pr_566.h"


/* This test will check the basic actions from the pthread API
 * which rely on clone(), exit() (NB:SYS_exit).
 */
void *__pthread_basic_child_self_ptr = NULL;

static void *pthread_basic_thread_fun(void *args)
{
	
	if ( args == (void*) 0xdeadbeef) {
		PRINT_GOOD("Argument arrived correctly in thread_fun()");
	} else {
		PRINT_BAD("Wrong argument in thread_fun %p != 0xdeadbeef", args);
	}
	__pthread_basic_child_self_ptr = pthread_self();
	
	return (void *) 0xbadbabee;
}

void test_pthread_basic()
{
	pthread_t thread_id;
	void *retval;

	/* this will invoke clone()*/
	pthread_create(&thread_id, NULL, pthread_basic_thread_fun,(void *) 0xdeadbeef);
	/* this will wait for the thread to do exit()*/
	pthread_join(thread_id, &retval);

	/* the address in thread_id is not changed after pthread_join*/
	if (__pthread_basic_child_self_ptr == (void *) thread_id) {
		PRINT_GOOD("Self reference for pthread threads works");
	} else {
		PRINT_BAD("Self reference for pthread threads doesn't work");
		PRINT_BAD("\t thread_id:%p != child_self_ptr:%p", thread_id, __pthread_basic_child_self_ptr);
	}

	if (retval == (void *) 0xbadbabee) {
		PRINT_GOOD("The returned value from the thread_fun si correct");
	} else {
		PRINT_BAD("Wrong returnd value:%p != 0xbadbabee", retval);
	}
}

