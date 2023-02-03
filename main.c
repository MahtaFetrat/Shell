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
    char **argv = (char **)malloc(MAX_CMD_ARG_COUNT * sizeof(char *));
    argv[0] = (char *)malloc(MAX_CMD_ARG_LEN * sizeof(char *)); // First arg is allocted for path.

    initialize_parser();
    initialize_executor();

    char input[MAX_CMD_ARG_COUNT * (MAX_CMD_ARG_LEN + 2)];
    int quitted = 0;

    while (!quitted)
    {
        print_prompt();
        if (scanf("%[^\n]%*c", input) != 0) // Input line (including white-spaces).
        {
            int cmd_code = parse(input, &argc, argv);

            if (cmd_code == 0) // quit cmd
                quitted = 1;
            else if (cmd_code == -1)
                printf("Command not found\n");
            else
                execute_command(cmd_code, argc, argv);
        }
        else
            scanf("%*c");   // Consume newline character.
        add_command_to_history(input);
    };
    destroy_parser();
    destroy_executor();

    free(argv);

    return 0;
}