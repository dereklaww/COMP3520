/* Main include header file for FCFS dispatcher */
#ifndef MLQ_MAIN
#define MLQ_MAIN

/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include "pcb.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif
#endif

#define PRINT_DEBUG 1

enum scheduler {
    L0_FCFS,
    L1_RR,
    L2_FCFS
};
