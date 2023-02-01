#include "stdio.h"
#include "stdlib.h"
#include "parser.h"
#include "executor.h"
#include <unistd.h>
#include <string.h>

void print_prompt()
{
    char prompt[200];
    getcwd(prompt, 200);

    char *home = getenv("HOME");
    int home_len = strlen(home);

    if (strncmp(prompt, home, home_len) == 0)
        printf("~%s$ ", prompt + home_len);
    else
        printf("%s$ ", prompt);
}

int main()
{
    int argc;
    // Initialize placeholder array for arguments of the input commands.
    char **argv = (char **)malloc(MAX_CMD_ARG_COUNT * sizeof(char *));
    for (int i = 0; i < MAX_CMD_ARG_COUNT; i++)
        argv[i] = (char *)malloc(MAX_CMD_ARG_LEN * sizeof(char));

    initialize_parser();
    initialize_executor();

    char input[MAX_CMD_ARG_COUNT * (MAX_CMD_ARG_LEN + 2)];
    int quitted = 0;

    while (!quitted)
    {
        print_prompt();
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
    destroy_executor();

    for (int i = 0; i < MAX_CMD_ARG_COUNT; i++)
        free(argv[i]);
    free(argv);

    return 0;
}