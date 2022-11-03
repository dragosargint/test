#include "test_pr_566.h"

/* This test will check the clone syscall behaves correctly */

extern int main_thread_tid;
int __clone_child_thread_tid;
unsigned long __clone_child_tls_ptr;
unsigned long __clone_child_tls_area;
unsigned long __clone_child_tls_ptr;
struct uk_thread *__clone_child_uk_thread_ptr = NULL;
__thread unsigned long per_thread_var = 0xdeadbeef;

static int clone_thread_fun(void *arg)
{
	__clone_child_uk_thread_ptr = uk_thread_current();
	unsigned long per_thread_var_addr = (unsigned long) &per_thread_var;
	unsigned long tls_start;
	unsigned long tls_end;

	__clone_child_thread_tid = syscall(SYS_gettid);
	if (__clone_child_thread_tid != main_thread_tid) {
		PRINT_GOOD("Tid assignation works, child_tid:%d != main_thread_tid:%d", __clone_child_thread_tid, main_thread_tid);
	} else {
		PRINT_BAD("Tid assignation doesn't work. child_tid:%d == main_thread_tid:%d", __clone_child_thread_tid, main_thread_tid);
	}

	if (__clone_child_tls_ptr == (unsigned long) ukplat_tlsp_get()) {
		PRINT_GOOD("Child TLS is set correctly to 0x%lx", __clone_child_tls_ptr);
	} else {
		PRINT_BAD("Child TLS:0x%lx != ukplat_tlsp_get():0x%lx", __clone_child_tls_ptr, ukplat_tlsp_get());
	}

	tls_start = __clone_child_tls_area;
	tls_end = __clone_child_tls_area + (unsigned long) ukarch_tls_area_size();
	if ( per_thread_var_addr <= tls_end && per_thread_var_addr >= tls_start) {
		PRINT_GOOD("per_thread_var is placed correctly in the TLS section of CHILD");
		PRINT_GOOD("\ttls_start:0x%lx <= &per_thread_var:0x%lx <= tls_end:0x%lx", tls_start, per_thread_var_addr, tls_end);
	} else {
		PRINT_BAD("per_thread_var is NOT placed correctly in the TLS sectio of CHILD");
		PRINT_BAD("\t&per_thread_var:0x%lx NOT in [tls_start:0x%lx, tls_end:0x%lx]", per_thread_var_addr, tls_start, tls_end);
	}

#ifdef CONFIG_ARCH_X86_64
#define TLSP_CMP_OP <=
#endif
#ifdef CONFIG_ARCH_ARM_64
#define TLSP_CMP_OP >=
#endif
	if ( per_thread_var_addr TLSP_CMP_OP __clone_child_tls_ptr) {
		PRINT_GOOD("per_thread_var is placed on the correct side of the tlsp");
		PRINT_GOOD("\t&per_thread_var:0x%lx " STRINGIFY(TLSP_CMP_OP) " tlsp:0x%lx", per_thread_var_addr, __clone_child_tls_ptr);
	} else {
		PRINT_BAD("per_thread_var is placed on the wrong side of tlsp");
		PRINT_BAD("\t&per_thread_var:0x%lx NOT " STRINGIFY(TLSP_CMP_OP) " tlsp:0x%lx", per_thread_var_addr, __clone_child_tls_ptr);
	}
#undef TLSP_CMP_OP

	per_thread_var = 0xbadbabee;

	return 0;
}


void test_clone()
{
	/* In this test we're trying to do what musl does with the clone syscall. */

	size_t stack_size = 8 * 4096; // not using the default size
	size_t tls_size = ukarch_tls_area_size();
	__u8 *map = mmap(NULL, stack_size + tls_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	__clone_child_tls_area = map + stack_size;
	void *tcb = __uk_copy_tls((void*) __clone_child_tls_area); // function defined in __uk_init_tls.c glue src file (musl)
	void *stack = map +  stack_size;
	int ctid;

	/* these flags are used by pthread_create() when a new thread is createad */
	unsigned flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND
                | CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS
                | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID | CLONE_DETACHED;

	__clone_child_tls_ptr = (unsigned long) ukarch_tls_tlsp(__clone_child_tls_area);
	/* Use the clone wrapper from musl.
	 * NB: this will call SYS_exit after clone_thread_fun() returns
	 */
	// __clone(func, stack, flags, arg, ptid, tls, ctid)
	__clone(clone_thread_fun, stack, flags, tcb, &ctid, __clone_child_tls_ptr, &ctid);
	/* clone will force the execution of child after it creates it,
         * but just to be sure the child executes and finishes,
	 * we place some sleeps here.
	 */
	unsigned int no_of_tries = 5;
	while (no_of_tries--) {
		if (!__check_if_child_exited(__clone_child_uk_thread_ptr))
			sleep(1);
		else
			break;
	}

	if (!__check_if_child_exited(__clone_child_uk_thread_ptr)) {
		PRINT_BAD("VERY BAD the child didn't exit, let's commit suicide");
		UK_CRASH("CHILD didn't exit");
	}

	/* At this point the child exited, so we need to check if the tid is set to 0.
	 * This must happen because one of the flags that we gave to clone() is CLONE_CHILD_CLEARTID
	 */
	if (ctid != 0 || ctid == __clone_child_thread_tid) {
		PRINT_BAD("CLONE_CHILD_CLEARTID has no effect, ctid:%d != 0", ctid);
	} else {
		PRINT_GOOD("CLONE_CHILD_CLEARTID works, ctid:%d == 0", ctid);
	}

	/* check if per_thread_var is really per thread */
	if (per_thread_var == 0xdeadbeef) {
		PRINT_GOOD("per_thread_var is indeed a per_thread_variable");
	} else {
		PRINT_BAD("per_thread_var:0x%lx != 0xdeadbeef was changed", per_thread_var);
	}
}
