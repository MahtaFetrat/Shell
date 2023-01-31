#include "parser.h"
#include <regex.h>
#include <stdlib.h>
#include <string.h>

const int MAX_CMD_ARG_COUNT = 50;
const int MAX_CMD_ARG_LEN = 100;

static const char *COMMANDS[] = {"quit"};

int command_count;
regex_t *compiled_regexes;

void initialize_parser()
{
    command_count = sizeof(COMMANDS) / sizeof(COMMANDS[0]);
    compiled_regexes = (regex_t *)malloc(command_count * sizeof(regex_t));

    for (int i = 0; i < command_count; i++)
    {
        regcomp(&(compiled_regexes[i]), COMMANDS[i], REG_EXTENDED);
    }
}

void extract_cmd_args(char *input, char **cmd_args, regmatch_t match_indices[])
{
    int i = 0;
    while (match_indices[i].rm_so != -1)
    {
        int arg_len = match_indices->rm_eo - match_indices->rm_so;
        strncpy(cmd_args[i], input + match_indices->rm_so, arg_len);
        cmd_args[i][match_indices->rm_eo] = 0;
        i++;
    }
    cmd_args[i] = NULL;
}

int parse(char *input, char **cmd_args)
{
    regmatch_t match_indices[MAX_CMD_ARG_COUNT];

    for (int i = 0; i < command_count; i++)
    {
        if (regexec(&(compiled_regexes[i]), input, MAX_CMD_ARG_COUNT, match_indices, 0) == 0)
        {
            extract_cmd_args(input, cmd_args, match_indices);
            return i;
        }
    }

    return -1;
}

void destroy_parser()
{
    free(compiled_regexes);
}