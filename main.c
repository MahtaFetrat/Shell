#include "stdio.h"
#include "stdlib.h"
#include "parser.h"

int main()
{
    // Initialize placeholder array for arguments of the input commands.
    char **cmd_args = (char **)malloc(MAX_CMD_ARG_COUNT * sizeof(char *));
    for (int i = 0; i < MAX_CMD_ARG_COUNT; i++)
        cmd_args[i] = (char *)malloc(MAX_CMD_ARG_LEN * sizeof(char));

    initialize_parser();

    char input[MAX_CMD_ARG_COUNT * (MAX_CMD_ARG_LEN + 2)];

    do
    {
        scanf("%s", input);
        int cmd_code = parse(input, cmd_args);
        printf("Input command: %d\n", cmd_code);
    } while (1);

    destroy_parser();

    // Free placeholder array for arguments of the input commands.
    for (int i = 0; i < MAX_CMD_ARG_COUNT; i++)
        free(cmd_args[i]);
    free(cmd_args);

    return 0;
}