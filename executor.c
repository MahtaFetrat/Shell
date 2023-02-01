#include "executor.h"
#include <stdio.h>

void print_help()
{
    printf("Help will be implemented here soon ...\n");
}

void run(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
        printf("arg: %s\n", argv[i]);
}

void execute_command(int cmd_code, int argc, char **argv)
{
    if (cmd_code == 1) // help
        print_help();
    else if (cmd_code == 2) // run command
        run(argc, argv);
}