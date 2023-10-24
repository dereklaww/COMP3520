# How does the program work?

1. Populate the FCFS queue based on the text file provided.

2. Ask the user to specify input:
- Level 0 time quantum
- Level 1 time quantum
- Level 1 maximum iterations

3. Multilevel dispatcher algorithm

While there's anything in any of the queues (i.e. dispatch/ level 0/ level 1/ level 2)
i. Unload any pending processes from the input queue:
While (the head of dispatch queue <= dispatcher timer)
equeue process from dispatch queue and enqueue on level 0 fcfs queue;

ii. if a process is currently running:
    if it is in level 0:
        decrement the process's remaining cpu time variable by t0

        if remaining cpu time variable <= 0:
            send SIGINT to terminate process
            calculate and accumulate turnaround time and wait time
            free up process structure memory
        else:
            send SIGTSTP to suspend process
            enqueue it on level 1 rr queue

    else if it is in level 1:

        decrement the process's remaining cpu time variable by t1
        increment the process's number of iterations in rr

        if remaining cpu time variable <= 0:
            send SIGINT to terminate process
            calculate and accumulate turnaround time and wait time
            free up process structure memory
        else:
            if iterations > k
                send SIGTSTP to suspend process
                enqueue it on level 2 fcfs queue
            else if there are other processes waiting in rr queue
                suspend and enqueue it back on rr queue

    else if it is in level 2:
        decrement the process's remaining cpu time variable by 1

        if remaining cpu time variable <= 0:
            send SIGINT to terminate process
            calculate and accumulate turnaround time and wait time
            free up process structure memory
            set level_2_process placeholder to NULL

iii. If no process currently running:
    a. check if there are processes waiting on level 0 fcfs and level 1 rr queue:
        if there are:
            dequeue and start process
        else:
    b. check if level_2_process placeholder is empty and there are processes waiting on level 2 fcfs queue:
        if there are:
            dequeue and start process
        else if level_2_process is not NULL:
            continue level_2_process
    c. set time quantum based on which queue the process is dequeued from.
    d. set next scheduler based on which queue the process is dequeued from.
        
iv. sleep for quantum;
    quantum logic:
    if there is a current running process and remaining service time is less than time quantum:
        quantum = remaining service time
    else if there are no running process:
        quantum = 1
    else:
        quantum = time quantum of current scheduler

v. Increment dispatcher timer and set current scheduler to next scheduler

vi. Go back to 3

vii. When all queues are empty, print out average turnaround time and average wait time.