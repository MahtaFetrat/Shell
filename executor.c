#include "executor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_ENV_PATHS 50
#define MAX_PATH_LEN 200

char **env_paths;

void tokenize_env_paths(char *PATH, char **paths)
{
    char *saveptr;
    paths[0] = strtok_r(PATH, ":", &saveptr);

    int i = 0;
    while (paths[i] != NULL)
    {
        paths[++i] = strtok_r(NULL, ":", &saveptr);
    }
}

void initialize_executor()
{
    // Allocate str placeholder for environmental paths
    env_paths = (char **)malloc(MAX_ENV_PATHS * sizeof(char *));
    for (int i = 0; i < MAX_ENV_PATHS; i++)
        env_paths[i] = (char *)malloc(MAX_PATH_LEN * sizeof(char));

    char *PATH = getenv("PATH");
    tokenize_env_paths(PATH, env_paths);
}

void print_help()
{
    printf("Help will be implemented here soon ...\n");
}

int set_executable_path(char **argv)
{
    if (strchr(argv[0], '/')) // Absolute or Relative path
    {
        if (access(argv[0], F_OK) != 0)
        {
            printf("Exection failed: No such file or directory");
            return 1;
        }
        else if (access(argv[0], X_OK) != 0)
        {
            printf("Exection failed: Permission denied");
            return 2;
        }
        return 0;
    }
    else // Lookup in the $PATH directories
    {
        int i = 0;
        char concatenated_path[200];
        while (env_paths[i] != NULL)
        {
            sprintf(concatenated_path, "%s/%s", env_paths[i], argv[0]);
            if (access(concatenated_path, X_OK) == 0)
            {
                strcpy(argv[0], concatenated_path);
                return 0;
            }
            i++;
        }
        // Neither a path nor a file found in $PATH
        printf("%s: command not found\n", argv[0]);
        return 3;
    }
}

void exec(char **argv)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Failed to create new process");
    }
    else if (pid == 0)
    {
        if (execv(argv[0], argv))
        {
            perror("Execution failed");
        }
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}

void run_program(int argc, char **argv)
{
    if (set_executable_path(argv) == 0)
        exec(argv);
}

void execute_command(int cmd_code, int argc, char **argv)
{
    if (cmd_code == 1) // help
        print_help();
    else if (cmd_code == 2) // run command
        run_program(argc, argv);
}

void destroy_executor()
{
    for (int i = 0; i < MAX_ENV_PATHS; i++)
        free(env_paths[i]);
    free(env_paths);
}