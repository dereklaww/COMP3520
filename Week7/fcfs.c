/*
    COMP3520 Exercise 4 - FCFS Dispatcher
    usage:
    ./fcfs <TESTFILE>
    where <TESTFILE> is the name of a job list
    */

/* Include files */
#include "fcfs.h"

void waitingTime(int *wt, int *bt, int n) {
    wt[0] = 0;
	for (int i = 1; i < n ; i++ )
		wt[i] = bt[i-1] + wt[i-1] ;
}
void turnAroundTime(int n, int *bt, int *wt, int *tat) //function to find the waitingtime
{
	for (int i = 0; i < n ; i++)
		tat[i] = bt[i] + wt[i];
}

int main (int argc, char *argv[])
{
    /*** Main function variable declarations ***/
    FILE * input_list_stream = NULL;
    PcbPtr fcfs_queue = NULL;
    PcbPtr current_process = NULL;
    PcbPtr process = NULL;
    int timer = 0;
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
        fcfs_queue = enqPcb(fcfs_queue, process);
        num_process++;

    }

    int *arrival_time_arr = malloc(num_process * sizeof(int));
    int *burst_time_arr = malloc(num_process * sizeof(int));
    int current_process_id = 0;

    // 2. Whenever there is a running process or the FCFS queue is not empty:
    while (current_process || fcfs_queue)
    {
        // i. If there is a currently running process;
        if (current_process)
        {
            // a. Decrement the process's remaining_cpu_time variable;
            current_process->remaining_cpu_time--;
            arrival_time_arr[current_process_id] = current_process->arrival_time;
            burst_time_arr[current_process_id] = current_process->service_time;
             
            // b. If the process's allocated time has expired:
            if (current_process->remaining_cpu_time <= 0)
            {
                current_process_id++;
                // A. Terminate the process;
                terminatePcb(current_process);
                // B. Deallocate the PCB (process control block)'s memory
                free(current_process);
                current_process = NULL;
            }
        }

        // ii. If there is no running process and there is a process ready to run:
        if (!current_process && fcfs_queue && fcfs_queue->arrival_time <= timer)
        {
            // Dequeue the process at the head of the queue, set it as currently running and start it
            current_process = deqPcb(&fcfs_queue);
            startPcb(current_process);
        }
        
        // iii. Let the dispatcher sleep for one second;
        sleep(1);
        // iv. Increment the dispatcher's timer;
        timer++;
        // v. Go back to 2.
    }
    // 3. Terminate the FCFS dispatcher

    int *waiting_time_arr = malloc(num_process * sizeof(int));
    int *turn_arround_arr = malloc(num_process * sizeof(int));
    int total_wt = 0;
    int total_tat = 0;

    waitingTime(waiting_time_arr, burst_time_arr, num_process);
    turnAroundTime(num_process, burst_time_arr, waiting_time_arr, turn_arround_arr);

    for (int i = 0; i < num_process ; i++ ) {
		total_wt = total_wt + waiting_time_arr[i];
        total_tat = total_tat + turn_arround_arr[i];
    }

    float a_wt = total_wt/num_process;
    float a_tat = total_tat/num_process;

    printf("Average Waiting Time = %0.4f, Average Turnaround Time = %0.4f\n", a_wt, a_tat);


    exit(EXIT_SUCCESS);
}

