/* Adaptive syscall for process exit */
/* This is the Linux x86_64-specific part of the shutdown module */

/* Define the system call number for exit */
#define SYS_exit 60

/* Inline function to make exit syscall */
static inline long syscall1_exit(long n, long a1)
{
	long ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(n), "D"(a1)
						 : "rcx", "r11", "memory");
	return ret;
}

/* Platform-specific exit function */
void platform_shutdown_exit(int exit_code)
{
	syscall1_exit(SYS_exit, (long)exit_code) /* Make actual exit syscall */;
}