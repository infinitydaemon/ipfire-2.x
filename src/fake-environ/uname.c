#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syslog.h>
#include <sys/utsname.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void *)-1l)
#endif

typedef int (*uname_t)(struct utsname *);

static void *get_libc_func(const char *funcname) {
    void *func = dlsym(RTLD_NEXT, funcname);
    if (!func) {
        syslog(LOG_ERR, "Failed to locate libc function %s: %s", funcname, dlerror());
        _exit(EXIT_FAILURE);
    }
    return func;
}

int uname(struct utsname *buf) {
    char *env = NULL;

    uname_t real_uname = get_libc_func("uname");
    int ret = real_uname(buf);

    if (ret < 0) {
        syslog(LOG_ERR, "Failed to get system information: %m");
        return ret;
    }

    /* Replace release if requested. */
    if ((env = getenv("UTS_RELEASE")) != NULL) {
        strncpy(buf->release, env, _UTSNAME_RELEASE_LENGTH);
    }

    /* Replace machine type if requested. */
    if ((env = getenv("UTS_MACHINE")) != NULL) {
        strncpy(buf->machine, env, _UTSNAME_MACHINE_LENGTH);
    }

    return ret;
}
