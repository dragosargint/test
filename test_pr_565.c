#include <uk/syscall.h>
#include <uk/print.h>

void call_binary_fork();

#define PRINT_GOOD(...)	\
do {	\
	uk_pr_err("GOOD: ");	\
	uk_pr_err(__VA_ARGS__);	\
	uk_pr_err("\n");	\
} while(0);


#define PRINT_BAD(...)	\
do {	\
	uk_pr_err("BAD: ");	\
	uk_pr_err(__VA_ARGS__);	\
	uk_pr_err("\n");	\
} while(0);


char TLS_REGION[100];
unsigned long return_addr;
int is_binary;
/*
 * This syscall is only for testing the User land TLS
 * Not a real fork !!!
 */
UK_SYSCALL_R_DEFINE(int, fork)
{
	if(is_binary) {
		struct uk_thread *self = uk_thread_current();
		if (self->uktlsp == (void *) ukplat_tlsp_get()) {
			PRINT_GOOD("KTLS base is at %p", self->uktlsp);
		} else {
			PRINT_BAD("KTLS 0x%lx != to self->uktlsp %p", ukplat_tlsp_get(), self->uktlsp);
			return 0;
		}
	
		if ((void *)ukplat_tlsp_get() != TLS_REGION) {
			PRINT_GOOD("KTLS 0x%lx != ULTLS %p", ukplat_tlsp_get(), TLS_REGION);
		} else {
			PRINT_BAD("KTLS 0x%lx == ULTLS %p", ukplat_tlsp_get(), TLS_REGION);
			return 0;
		}
		
		if ((void *) uk_syscall_ultlsp() == TLS_REGION) {
			PRINT_GOOD("uk_syscall_ultlsp() gives %p of ULTLS", TLS_REGION);
		} else {
			PRINT_BAD("uk_syscall_ultlsp() %p != ULTLS %p", uk_syscall_ultlsp(), TLS_REGION);
		}
	}

	if (return_addr == uk_syscall_return_addr()) {
		PRINT_GOOD("The return address is saved correctly 0x%lx", return_addr);
	} else {
		PRINT_BAD("return_addr 0x%lx != uk_syscall_return_addr() 0x%lx", return_addr, uk_syscall_return_addr());
	}
	return 0;
}


void test_pr_565()
{
	/* mimics an userland tls */
	ukplat_tlsp_set(TLS_REGION);
	
	if ((void *) ukplat_tlsp_get() == TLS_REGION) {
		PRINT_GOOD("ULTLS base is at %p", TLS_REGION);
	} else {
		PRINT_BAD("ULTLS is different. we set %p but got 0x%lx", TLS_REGION, ukplat_tlsp_get());
	}
	is_binary = 1;
	call_binary_fork();
	/*when we return from syscall the ULTLS pointer should be restored */
	if ((void *) ukplat_tlsp_get() == TLS_REGION) {
		PRINT_GOOD("ULTLS is at %p after syscall return", TLS_REGION);
	} else {
		PRINT_BAD("ULTLS %p != 0x%lx after syscall return", TLS_REGION, ukplat_tlsp_get());
	}

	is_binary = 0;
	call_fork();
}
