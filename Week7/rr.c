/*
COMP3520 Assignment 2 - RR Dispatcher
usage:
./rr <TESTFILE>
where <TESTFILE> is the name of a job list
*/
/* Include files */

#include "rr.h"

int main (int argc, char *argv[])
{
    /*** Main function variable declarations ***/
    FILE * input_list_stream = NULL;
    PcbPtr job_queue = NULL;
    PcbPtr rr_queue = NULL;
    PcbPtr current_process = NULL;
    PcbPtr process = NULL;
    int timer = 0;
    int time_quantum;
    int quantum;
    int turnaround_time;
    double av_turnaround_time = 0.0, av_wait_time = 0.0;
    int n = 0;

    // 1. Populate the RR queue
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
    while (!feof(input_list_stream)) { // put processes into job_queue
        process = createnullPcb();
        if (fscanf(input_list_stream,"%d, %d",
        &(process->arrival_time), &(process->service_time)) != 2) {
            free(process);
            continue;
        }
        process->remaining_cpu_time = process->service_time;
        process->status = PCB_INITIALIZED;
        job_queue = enqPcb(job_queue, process);
        n++;
    }

    // 2. Ask the user to specify a time quantum
    printf("Please enter a positive integer for the time quantum: ");
    scanf("%d", &time_quantum);
    if (time_quantum <= 0)
    {
        printf("Time quantum must be greater than 0.\n");
        exit(EXIT_FAILURE);
    }

    // 3. While there's anything in any of the queues or there is a currently running process:
    while (job_queue || rr_queue || current_process ) {
        // i. Unload any pending processes from the input queue:
        // While (head-of-input-queue.arrival-time <= dispatcher timer)
        // dequeue process from input queue and enqueue on RR queue;
        while (job_queue && job_queue->arrival_time <= timer) {
            process = deqPcb(&job_queue); // dequeue process
            process->status = PCB_READY; // set pcb ready
            rr_queue = enqPcb(rr_queue, process); // & put on RR queue
        }
        // ii. If a process is currently running;
        if (current_process) {
            // a. Decrement process remaining cpu time;
            current_process->remaining_cpu_time -= quantum;
            // b. If times up:
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
            // c. else if other processes are waiting in RR queue:
            } else if (rr_queue) {
                // A. Send SIGTSTP to suspend it;
                suspendPcb(current_process);
                // B. Enqueue it back on RR queue;
                rr_queue = enqPcb(rr_queue, current_process);
                current_process = NULL;
            }
        }
        // iii. If no process currently running && RR queue is not empty:
        if (!current_process && rr_queue) {
            // a. Dequeue process from RR queue
            current_process = deqPcb(&rr_queue);
            // b. If already started but suspended, restart it (send SIGCONT to it)
            // else start it (fork & exec)
            // c. Set it as currently running process;
            startPcb(current_process);
        }
        // iv. sleep for quantum;
        quantum = current_process && current_process->remaining_cpu_time <
        time_quantum ?
        current_process->remaining_cpu_time :
        !(current_process) ? 1 : time_quantum;
        sleep(quantum);
        // v. Increment dispatcher timer;
        timer += quantum;
        // vi. Go back to 3.
    }
    // print out average turnaround time and average wait time
    av_turnaround_time = av_turnaround_time / n;
    av_wait_time = av_wait_time / n;
    printf("average turnaround time = %f\n", av_turnaround_time);
    printf("average wait time = %f\n", av_wait_time);
    // 4. Terminate the RR dispatcher
    exit(EXIT_SUCCESS);
}
