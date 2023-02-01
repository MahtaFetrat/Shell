#include "executor.h"
#include <stdio.h>
#include<unistd.h>
#include <sys/wait.h>


void print_help()
{
    printf("Help will be implemented here soon ...\n");
}

void run(int argc, char **argv)
{
    pid_t pid = fork ();
    if (pid < 0) { 
        perror("Failed to create new process");
    }
    else if (pid==0) { 
        if (execv(argv[0], argv)) {
            perror ("Execution failed");
        }
    } else {
        waitpid(pid, NULL, 0);
    }
}


void execute_command(int cmd_code, int argc, char **argv)
{
    if (cmd_code == 1) // help
        print_help();
    else if (cmd_code == 2) // run command
        run(argc, argv);
}