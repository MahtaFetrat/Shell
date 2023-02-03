#include "stdio.h"
#include "stdlib.h"
#include "parser.h"
#include "executor.h"
#include <unistd.h>
#include <string.h>

#define MAX_COMMAND_LEN 20

char **predefined_command_keys;
char **predefined_command_values;
int command_num;

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

void read_predefined_commands()
{
    FILE *fp;
    fp = fopen("my_commands.txt", "r");

    predefined_command_keys = (char **)malloc(10 * sizeof(char *));
    predefined_command_values = (char **)malloc(10 * sizeof(char *));
    for (int i = 0; i < 10; i++)
    {
        predefined_command_keys[i] = (char *)malloc(MAX_COMMAND_LEN * sizeof(char));
        predefined_command_values[i] = (char *)malloc(MAX_COMMAND_LEN * sizeof(char));
    }

    int count = 0;
    char buffer[10][MAX_COMMAND_LEN];
    char str[MAX_COMMAND_LEN];
    while (EOF != fscanf(fp, "%30[^\n]\n", str))
    {
        strcpy(buffer[count], str);
        count++;
    }
    command_num = count;
    for (int i = 0; i < count; i++)
    {
        predefined_command_keys[i] = strtok(buffer[i], "::");
        predefined_command_values[i] = strtok(NULL, "::");
    }
    fclose(fp);
}

int main()
{

    int argc;
    char **argv = (char **)malloc(MAX_CMD_ARG_COUNT * sizeof(char *));
    argv[0] = (char *)malloc(MAX_CMD_ARG_LEN * sizeof(char *)); // First arg is allocted for path.

    read_predefined_commands();
    initialize_parser();
    initialize_executor();

    char input[MAX_CMD_ARG_COUNT * (MAX_CMD_ARG_LEN + 2)];
    int quitted = 0;

    while (!quitted)
    {
        print_prompt();
        if (scanf("%[^\n]%*c", input) != 0) // Input line (including white-spaces).
        {
            add_command_to_history(input);
            
            int flag = 0;
            int cmd_code;
            for (int i = 0; i < command_num; i++)
            {
                if (strcmp(input, predefined_command_keys[i]) == 0)
                {
                    cmd_code = parse(predefined_command_values[i], &argc, argv);
                    flag = 1;
                    break;
                }
            }
            if (flag == 0)
            {
                cmd_code = parse(input, &argc, argv);
            }

            if (cmd_code == 0) // quit cmd
                quitted = 1;
            else if (cmd_code == -1)
                perror("Command not found\n");
            else
                execute_command(cmd_code, argc, argv);
        }
        else
            scanf("%*c"); // Consume newline character.
    };
    destroy_parser();
    destroy_executor();

    free(argv);

    return 0;
}