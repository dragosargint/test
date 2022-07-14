#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <uk/print.h>
int __libc_start_main(int (*main)(int,char **,char **), int argc, char **argv);
int real_main();

int main(int argc, char **argv)
{
	uk_pr_err("this must be 2");
	__libc_start_main(real_main, argc, argv);
}

void *fun(void *vargp)
{
    sleep(1);
    printf("Hello from Thread \n");
    fflush(stdout);
    return NULL;
}
/*UK_LLSYSCALL_R_DEFINE2(int, clone,
                       unsigned long, flags,
                       void *, sp,
                       int *, parent_tid,
                       int *, child_tid,
                       unsigned long, tlsp)
 */  

#include <uk/tcb_impl.h>

int uk_thread_uktcb_init(void *uktcb)
{
  struct pthread_tcb *tcb = (struct pthread_tcb *) uktcb;

  UK_ASSERT(uktcb);

  /* your init (e.g. memset()) */

  uk_pr_err("this must be 1\n");
  return 0; /* >= 0 means success, < 0 means error [try using errnos]*/
}

void uk_thread_uktcb_fini(void *uktcb)
{
  UK_ASSERT(uktcb);
  uk_pr_err("here we are\n");

  /* you uninit */
}


int real_main()
{
    pthread_t thread_id;
    printf("Before Thread %p\n", pthread_self());
    fflush(stdout);
    pthread_create(&thread_id, NULL, fun, NULL);
    pthread_join(thread_id, NULL);
    printf("After Thread\n");
    fflush(stdout);
    return 0;
    uk_syscall_r_clone(0, NULL, NULL, NULL, 0);
//	uk_syscall_r_clone(0,"hello", 5);
	return 0;
}
