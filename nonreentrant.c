/**
 * tlpi Example 21-1
 * author:fanxiangchao
 **/

#define _XOPEN_SOURCE 600
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "tlpi_hdr.h"

static char *str2;     /* Set from argv[2] */
static int handled = 0;   /* Counts number of calls to handler */

static void handler(int sig)
{
    crypt(str2,"xx");
    handled++;
}

int main(int argc,char *argv[])
{
    char *cr1;
    int callNum,mismatcch;
    struct sigaction sa;

    if (argc != 3)
    {
        usageErr("%s str1 str2\n",argv[0]);
    }

    str2 = argv[2];   /* Make argv[2] avaiable to handler */
    cr1 = strdup(crypt(argv[1],"xx"));   /* Copy statically allocated string to another buffer */

    if (cr1 == NULL)
    {
        errExit("strdup");
    }

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGINT,&sa,NULL) == -1)
    {
        errExit("sigaction");
    }

    /* Repeatedly call crypt() using argv[1]. If interrupted by a signal handler,
       then the static storage returned by crypt() will be ovverwritten by then
       results of encrypting argv[2],and strcmp() will detect a mismatch with
       the value in 'cr1'.
     */
    for (callNum = 1,mismatcch = 0; ; callNum++)
    {
        if (strcmp(crypt(argv[1],"xx"),cr1) != 0)
        {
            mismatcch++;
            printf("Mismatch on call %d (mismatch = %d handled = %d)\n",callNum,mismatcch,handled);
        }
    }
    
    
}
