#define _GNU_SOURCE
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sched.h>
#include "tlpi_hdr.h"

#ifndef CHILD_SIG
#define CHILD_SIG SIGUSR1 /* Signal to be generated on termination
                             of cloned child */
#endif

static int            /* Startup function for cloned child */
childFunc(void *arg)
{
    if (close(*((int *)arg)) == -1)
    {
        errExit("close");
    }

    return 0;            /* Child terminates noew */
}

int main(int argc,char *argv[])
{
    const int STACK_SIZE = 65536;    /* Stack size for cloned child */
    char *stack;                     /* Start of stack buffer */
    char *stackTop;                  /* End of stack buffer */
    int s,fd,flags;

    fd = open("/dev/null",O_RDWR);  /* Child will close this fd */
    if (fd == -1)
    {
        errExit("open");
    }

    /* If argc > 1, child shares file descriptor table with parent */

    flags =  (argc > 1) ? CLONE_FILES : 0;

    /* Allocate stack for child */

    stack  = malloc(STACK_SIZE);
    if (stack == NULL)
    {
        errExit("malloc");
    }

    stackTop = stack + STACK_SIZE;       /* Assume stack grows downward */

    /* Ignore CHILD_SIG, in case it is a signal whose default is to
       terminate the process; but don't ignore SIGCHLD (which is ignored
       by default),since that would prevent the creation of za zombie. */

    if (CHILD_SIG != 0 && CHILD_SIG != SIGCHLD)
    {
        if (signal(CHILD_SIG,SIG_IGN) ==  SIG_ERR)
        {
            errExit("signal");
        }
    }

    /* Create child; child commences execution in childFunc() */
    if (clone(childFunc,stackTop,flags | CHILD_SIG,(void *)&fd) == -1)
    {
        errExit("clone");
    }

    /* Parent falls through to here. Wait for child; __WCLONE is
       need for child notifying with signal other than SIGCHLD. */

    if (waitpid(-1,NULL,(CHILD_SIG != SIGCHLD) ? __WCLONE : 0) == -1)
    {
        errExit("waitpid");
    }

    printf("child has terminated\n");

    /* Did close() of file descriptor in child affect parent? */
    s = write(fd,"x",1);
    if (s == -1 && errno == EBADF)
    {
        printf("file descriptor %d has been closed\n",fd);
    }else if (s == -1)
    {
        printf("write() on file descriptor %d failed "
               "unexpectedly (%s)\n",fd,strerror(errno));
    }else
    {
        printf("write() on file descriptor %d succeeded\n",fd);
    }

    exit(EXIT_SUCCESS);
}
