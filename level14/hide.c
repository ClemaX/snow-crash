#define _GNU_SOURCE
#include <sys/ptrace.h>
#include <stdarg.h>
#include <unistd.h>

/* Ease development on macOS */
#ifdef __APPLE__
# define RTLD_NEXT 0

enum __ptrace_request
{ PTRACE_TRACEME };

#else
# include <dlfcn.h>
#endif

long	(*ptrace_orig)(int req, ...) = NULL;

long	ptrace(enum __ptrace_request req, ...)
{
	va_list	ap;

	va_start(ap, req);

	if (ptrace_orig == NULL)
		ptrace_orig = dlsym(RTLD_NEXT, "ptrace");

	if (req == PTRACE_TRACEME)
		return 0;

	va_end(ap);

	return ptrace_orig(req, va_arg(ap, int), va_arg(ap, void*), va_arg(ap, int), va_arg(ap, void*));
}
