#include "parser.h"
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const int MAX_CMD_ARG_COUNT = 50;
const int MAX_CMD_ARG_LEN = 100;

static const char *COMMANDS[] = {
    "^[[:space:]]*quit[[:space:]]*$",
    "^[[:space:]]*help[[:space:]]*$",
    "^[[:space:]]*cd[[:space:]]+[^[:space:]]+[[:space:]]*$", // cd [dir]
    "^[[:space:]]*cwd[[:space:]]*$",
    "^[[:space:]]*history[[:space:]]*$",
    "^[[:space:]]*[^[:space:]]+([[:space:]]+[^[:space:]><]*)*[[:space:]]+>[[:space:]]+[^[:space:]><]+[[:space:]]*$", // [program] [args ...] > [outfile]
    "^[[:space:]]*[^[:space:]]+([[:space:]]+[^[:space:]><]*)*[[:space:]]+<[[:space:]]+[^[:space:]><]+[[:space:]]*$", // [program] [args ...] < [infile]
    "^[[:space:]]*[^[:space:]]+([[:space:]]+[^[:space:]><]+)*[[:space:]]+&[[:space:]]*$",                            // [program] [args ...] &
    "^([[:space:]]*[^[:space:]]+([[:space:]]+[^[:space:]><]+)*[[:space:]]*;)+[[:space:]]*[^[:space:]]+([[:space:]]+[^[:space:]><]+)*[[:space:]]*$", // [program] [args ...]; [program] [args ...]; ...
    "^[[:space:]]*[^[:space:]]+([[:space:]]+[^[:space:]><]+)*[[:space:]]*$"};                                        // [program] [args ...]

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

void get_args(char *input, int *argc, char **argv)
{
    char *saveptr;
    strcpy(argv[0], strtok_r(input, " ", &saveptr));

    int i = 0;
    while (argv[i] != NULL)
    {
        argv[++i] = strtok_r(NULL, " ", &saveptr);
    }
    *argc = i;
}

int parse(char *input, int *argc, char **argv)
{
    for (int i = 0; i < command_count; i++)
    {
        if (regexec(&(compiled_regexes[i]), input, 0, NULL, 0) == 0)
        {
            get_args(input, argc, argv);
            return i;
        }
    }

    return -1;
}

void destroy_parser()
{
    free(compiled_regexes);
}