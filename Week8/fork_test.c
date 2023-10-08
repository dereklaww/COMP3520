#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    pid_t p;
    p = fork();

    if (p == 0) {
        printf("child process is running\n");
        char *args[] = {"./process",  NULL};
        execvp(args[0], args);
    }

    sleep(10);
    printf("parent process is running\n");
    if (kill(p, SIGINT)) fprintf(stderr, "terminate of %d failed", (int) p);

}