#include "stdio.h"
#include "stdlib.h"
#include "parser.h"
#include "executor.h"

int main()
{
    int argc;
    // Initialize placeholder array for arguments of the input commands.
    char **argv = (char **)malloc(MAX_CMD_ARG_COUNT * sizeof(char *));
    for (int i = 0; i < MAX_CMD_ARG_COUNT; i++)
        argv[i] = (char *)malloc(MAX_CMD_ARG_LEN * sizeof(char));

    initialize_parser();

    char input[MAX_CMD_ARG_COUNT * (MAX_CMD_ARG_LEN + 2)];
    int quitted = 0;

    while (!quitted)
    {
        scanf("%[^\n]%*c", input); // Input line (including white-spaces).
        int cmd_code = parse(input, &argc, argv);

        if (cmd_code == 0) // quit cmd
            quitted = 1;
        else if (cmd_code == -1)
            printf("Command not found\n");
        else
            execute_command(cmd_code, argc, argv);
    };

    destroy_parser();

    for (int i = 0; i < MAX_CMD_ARG_COUNT; i++)
        free(argv[i]);
    free(argv);

    return 0;
}