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
    // Set environment PATHs for execution.
    env_paths = (char **)malloc(MAX_ENV_PATHS * sizeof(char *));
    char *PATH = getenv("PATH");
    tokenize_env_paths(PATH, env_paths);

    // Change working directory to home.
    chdir(getenv("HOME"));
}

int set_executable_path(char **argv)
{
    if (strchr(argv[0], '/') != NULL) // Absolute or Relative path
    {
        if (access(argv[0], F_OK) != 0)
        {
            printf("Exection failed: No such file or directory\n");
            return 1;
        }
        else if (access(argv[0], X_OK) != 0)
        {
            printf("Exection failed: Permission denied\n");
            return 2;
        }
        return 0;
    }
    else // Lookup in the $PATH directories
    {
        int i = 0;
        char catenated_path[200];

        while (env_paths[i] != NULL)
        {
            sprintf(catenated_path, "%s/%s", env_paths[i], argv[0]);
            if (access(catenated_path, X_OK) == 0)
            {
                strcpy(argv[0], catenated_path);
                return 0;
            }
            i++;
        }
        // Neither a path nor a file found in $PATH
        printf("%s: Invalid use of command\n", argv[0]);
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

void run_program(char **argv)
{
    if (set_executable_path(argv) == 0)
        exec(argv);
}

void cwd()
{
    char cwd[200];
    getcwd(cwd, 200);
    printf("%s\n", cwd);
}

void cd(char **argv)
{
    // Replace ~ by HOME since non-recognized by chdir.
    if (strcmp(argv[1], "~") == 0 || (argv[1][0] == '~' && argv[1][1] == '/'))
    {
        char home_replaced[200];
        sprintf(home_replaced, "%s%s", getenv("HOME"), argv[1] + 1);
        strcpy(argv[1], home_replaced);
    }

    if (chdir(argv[1]) != 0)
    {
        printf("cd: Inaccessible directory\n");
    }
}

void print_help()
{
    printf("These shell commands are defined internally.  Type `help' to see this list.\n");
    printf("quit\t\t\tExit the shell\n");
    printf("cd [dir]\t\tChange the shell working directory.\n");
    printf("cwd\t\t\tPrint the name of the current working directory.\n");
    printf("[program] [args ...]\tExecute program with absolute/relative path or found in PATH dirs with the given args.\n");
    printf("help\t\t\tDisplay information about builtin commands.\n");
}

void execute_command(int cmd_code, int argc, char **argv)
{
    if (cmd_code == 1) // help
        print_help();
    else if (cmd_code == 2) // cd command
        cd(argv);
    else if (cmd_code == 3) // cwd command
        cwd();
    else if (cmd_code == 4) // run command
        run_program(argv);
}

void destroy_executor()
{
    for (int i = 0; i < MAX_ENV_PATHS; i++)
        free(env_paths[i]);
    free(env_paths);
}