#define _POSIX_C_SOURCE 199309
#include <signal.h>
#include <time.h>
#include "curr_time.h"   /* Declares currTime() */
#include "itimerspec_from_str.h" /* Declares itimerspecFromStr()*/
#include "tlpi_hdr.h"

#define TIMER_SIG SIGRTMAX   /* Our timer notification signal */

static void handler(int sig,siginfo_t *si,void *uc)
{
    timer_t *tidptr;

    tidptr= si->si_value.sival_ptr;

    /* UNSAFE: This handler uses non-async-signal-safe functions
       (printf();)*/
    printf("[%s] Got signal %d\n",currTime("%T"),sig);
    printf("     *sival_ptr         =%ld\n",(long)*tidptr);
    printf("     timer_getoverrun() =%d\n",timer_getoverrun(*tidptr));
}

int main(int argc,char *argv[])
{
    struct itimerspec ts;
    struct sigevent sev;
    timer_t *tidlist;
    siginfo_t si;
    sigset_t allSigs;
    int sig;
    int j;
    timer_t *tidptr;

    if (argc < 2)
    {
        usageErr("%s secs[/nsecs][:int-secs[/int-nsecs]]...\n",argv[0]);
    }

    tidlist = calloc(argc - 1,sizeof(timer_t));
    if (tidlist == NULL)
    {
        errExit("malloc");
    }

    sigfillset(&allSigs);
    if (sigprocmask(SIG_SETMASK,&allSigs,NULL) == -1)
    {
        errExit("sigprocmask");
    }

    printf("%s: signals blocked\n",argv[0]);
    
    /* Create and start one timer for each command-line argument */
    sev.sigev_notify = SIGEV_SIGNAL;  /* Notify via signal */
    sev.sigev_signo = TIMER_SIG;      /* Notify using this signal */

    for (j = 0; j < argc - 1; j++)
    {
        itimerspecFromStr(argv[j + 1],&ts);
        sev.sigev_value.sival_ptr = &tidlist[j];
        /* Allows handler to get ID of  this timer */

        if (timer_create(CLOCK_REALTIME,&sev,&tidlist[j]) == -1)
        {
            errExit("timer_create");
        }

        printf("Timer ID: %ld (%s)\n",(long)tidlist[j],argv[j + 1]);

        if (timer_settime(tidlist[j],0,&ts,NULL) == -1)
        {
            errExit("timer_settime");
        }
    }

    for (;;)          /* Wait for incoming timer signals */
    {
        sig = sigwaitinfo(&allSigs,&si);
        if (sig == -1)
        {
            errExit("sigwaitinfo");
        }

        if (sig == SIGINT || sig == SIGTERM)
        {
            exit(EXIT_SUCCESS);
        }

        if (sig == TIMER_SIG)
        {
            tidptr= si.si_value.sival_ptr;
            /* UNSAFE: This handler uses non-async-signal-safe functions
               (printf();)*/
            printf("[%s] Got signal %d\n",currTime("%T"),sig);
            printf("     *sival_ptr         =%ld\n",(long)*tidptr);
            printf("     timer_getoverrun() =%d\n",timer_getoverrun(*tidptr));

        }else
        {
            printf("other signals\n");
        }

    }
}