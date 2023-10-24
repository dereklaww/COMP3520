/***
 * COMP3520 - Assignment 2
 * SID: 510226244
 * ***/

#include "mlq.h"

#define PRINT_DEBUG 1

enum scheduler {
    L0_FCFS,
    L1_RR,
    L2_FCFS
};

int main (int argc, char *argv[])
{
    /*** Main function variable declarations ***/
    FILE * input_list_stream = NULL;
    PcbPtr dispatch_queue = NULL;
    PcbPtr rr_l1 = NULL;
    PcbPtr fcfs_l0 = NULL;
    PcbPtr fcfs_l2 = NULL;
    PcbPtr current_process = NULL;
    // PcbPtr next_process = NULL;
    PcbPtr l2_fcfs_process = NULL;
    PcbPtr process = NULL;
    int l0_tq = 0;
    int l1_tq = 0;
    int l1_iterations = 0;
    int current_schedular = -1;
    int next_schedular = -1;
    int time_quantum = 0;
    int quantum = 0;
    int timer = 0;
    int turnaround_time;
    double av_turnaround_time = 0.0, av_wait_time = 0.0;
    int num_process = 0;

    // 1. Populate the FCFS queue
    if (argc <= 0)
    {
        fprintf(stderr, "FATAL: Bad arguments array\n");
        exit(EXIT_FAILURE);
    }

    else if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <TESTFILE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!(input_list_stream = fopen(argv[1], "r")))
    {
        fprintf(stderr, "ERROR: Could not open \"%s\"\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    while (!feof(input_list_stream)) { // put processes into fcfs_queue
        process = createnullPcb();
        if (fscanf(input_list_stream,"%d, %d", &(process->arrival_time), &(process->service_time)) != 2) {
            free(process);
            continue;
        }

        process->remaining_cpu_time = process->service_time;
        process->status = PCB_INITIALIZED;
        dispatch_queue = enqPcb(dispatch_queue, process);
        num_process++;
    }

    // 2. Ask the user to specify run times
    printf("Please enter a positive integer for Level-0 time quantum: ");
    scanf("%d", &l0_tq);
    if (l0_tq <= 0)
    {
        printf("Level-0 time quantum must be greater than 0.\n");
        exit(EXIT_FAILURE);
    }

    printf("Please enter a positive integer for Level-1 time quantum: ");
    scanf("%d", &l1_tq);
    if (l1_tq <= 0)
    {
        printf("Level-1 time quantum must be greater than 0.\n");
        exit(EXIT_FAILURE);
    }

    printf("Please enter a positive integer for Level-1 iterations: ");
    scanf("%d", &l1_iterations);
    if (l1_iterations <= 0)
    {
        printf("Level-1 iterations must be greater than 0.\n");
        exit(EXIT_FAILURE);
    }

    // 3. While there's anything in any of the queues or there is a currently running process:
    while (dispatch_queue || fcfs_l0 || rr_l1 || fcfs_l2 || current_process ) {
        // i. Unload any pending processes from the input queue:
        // While (head-of-input-queue.arrival-time <= dispatcher timer)
        // dequeue process from input queue and enqueue on fcfs_l0 queue;
        while (dispatch_queue && dispatch_queue->arrival_time <= timer) {
            process = deqPcb(&dispatch_queue); // dequeue process
            process->status = PCB_INITIALIZED; // set pcb ready
            fcfs_l0 = enqPcb(fcfs_l0, process); // & put on fcfs_l0 queue
        }
        // ii. If a process is currently running;
        if (current_process) {

            #ifdef PRINT_DEBUG
            printf("curr schedular: %d\n", current_schedular);
            #endif

            switch (current_schedular) {

                case L0_FCFS:
                // a. Decrement the process's remaining_cpu_time variable;
                    current_process->remaining_cpu_time -= l0_tq;
                    
                    if (current_process->remaining_cpu_time <= 0) {
                        // A. Send SIGINT to the process to terminate it;
                        terminatePcb(current_process);
                        // Calculate and acumulate turnaround time and wait time;
                        turnaround_time = timer - current_process->arrival_time;
                        av_turnaround_time += turnaround_time;
                        av_wait_time += (turnaround_time - current_process->service_time);
                        printf ("turnaround time = %d, waiting time = %d\n", turnaround_time,
                        turnaround_time - current_process->service_time);
                        // B. Free up process structure memory
                        free(current_process);
                        current_process = NULL;
                    } else {
                        // A. Send SIGTSTP to suspend it;
                        suspendPcb(current_process);
                        // B. Enqueue it on RR queue;
                        rr_l1 = enqPcb(rr_l1, current_process);
                        current_process = NULL;
                    }  
                    break;

                case L1_RR:
                    current_process->remaining_cpu_time -= l1_tq;
                    current_process->no_iterations++;
                    
                    if (current_process->remaining_cpu_time <= 0) {
                        // A. Send SIGINT to the process to terminate it;
                        terminatePcb(current_process);
                        // Calculate and acumulate turnaround time and wait time;
                        turnaround_time = timer - current_process->arrival_time;
                        av_turnaround_time += turnaround_time;
                        av_wait_time += (turnaround_time - current_process->service_time);
                        printf ("turnaround time = %d, waiting time = %d\n", turnaround_time,
                        turnaround_time - current_process->service_time);
                        // B. Free up process structure memory
                        free(current_process);
                        current_process = NULL;
                    } else {
                        // if iterations > k
                        if (current_process->no_iterations >= l1_iterations){
                        // A. Send SIGTSTP to suspend it;
                        suspendPcb(current_process);
                        // B. Enqueue it on fcfs_l2 queue;
                        fcfs_l2 = enqPcb(fcfs_l2, current_process);
                        current_process = NULL;
                        } else if (rr_l1) {
                            suspendPcb(current_process);
                            // B. Enqueue it back on RR queue;
                            rr_l1 = enqPcb(rr_l1, current_process);
                            current_process = NULL;
                        }
                    }
                    break;
                
                case L2_FCFS:
                    // a. Decrement the process's remaining_cpu_time variable;
                    current_process->remaining_cpu_time--;
                    
                    // b. If the process's allocated time has expired:
                    if (current_process->remaining_cpu_time <= 0)
                    {
                        // A. Terminate the process;
                        terminatePcb(current_process);
                        turnaround_time = timer - current_process->arrival_time;
                        av_turnaround_time += turnaround_time;
                        av_wait_time += turnaround_time - current_process->service_time;
                        // B. Deallocate the PCB (process control block)'s memory
                        free(current_process);
                        current_process = NULL;
                        l2_fcfs_process = NULL;
                    }
                    break;
            }
        }

        // iii. If no process currently running:
        if (!current_process) {
            // a. Dequeue process
            if (fcfs_l0 && fcfs_l0->arrival_time <= timer) {
                time_quantum = l0_tq;
                current_process = deqPcb(&fcfs_l0);
                next_schedular = L0_FCFS;
            } else if (rr_l1) {
                time_quantum = l1_tq;
                current_process = deqPcb(&rr_l1);
                next_schedular = L1_RR;
            } else if (!l2_fcfs_process && fcfs_l2) {
                l2_fcfs_process = deqPcb(&fcfs_l2);
                time_quantum = 1;
                current_process = l2_fcfs_process;
                next_schedular = L2_FCFS;
            } else if (l2_fcfs_process){
                time_quantum = 1;
                current_process = l2_fcfs_process;
                next_schedular = L2_FCFS;
            }

            // b. If already started but suspended, restart it (send SIGCONT to it)
            // else start it (fork & exec)
            // c. Set it as currently running process;
            if (current_process) {
                startPcb(current_process);
            }
        }

        // iv. sleep for quantum;
        quantum = current_process && current_process->remaining_cpu_time <
        time_quantum ?
        current_process->remaining_cpu_time :
        !(current_process) ? 1 : time_quantum;

        #ifdef PRINT_DEBUG
        printf("quantum: %d\n", quantum);
        #endif

        sleep(quantum);
        // v. Increment dispatcher timer;
        timer += quantum;
        #ifdef PRINT_DEBUG
        printf("current timer: %d\n", timer);
        #endif
        current_schedular = next_schedular;
        // vi. Go back to 3.
    }
    // print out average turnaround time and average wait time
    av_turnaround_time = av_turnaround_time / num_process;
    av_wait_time = av_wait_time / num_process;
    printf("average turnaround time = %f\n", av_turnaround_time);
    printf("average wait time = %f\n", av_wait_time);
    // 4. Terminate the dispatcher
    exit(EXIT_SUCCESS);
}

