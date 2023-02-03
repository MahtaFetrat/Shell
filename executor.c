#include "executor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAX_ENV_PATHS 50
#define MAX_PATH_LEN 200
#define MAX_COMMANDS_LEN 1000

char **env_paths;
char command_history[MAX_COMMANDS_LEN];

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

/**
 * Converts any user input path for the program to some absolute executable path
 */
int set_executable_path(char **argv)
{
    if (strchr(argv[0], '/') != NULL) // Absolute or Relative path
    {
        if (access(argv[0], F_OK) != 0)
        {
            printf("%s\n", argv[0]);
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

int redirect_output(char *filepath) {
    int file = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (file < 0) {
        printf("Couldn't open output file\n");
        return 1;
    }
    
    if (dup2(file, 1) < 0 || dup2(file, 2) < 0)     // Redirect STDOUT and STDERR to file.
    {
        printf("Couldn't redirect output to file\n");
        return 2;
    }

    close(file);
    return 0;
}

void exec_redirect_output(int argc, char **argv) {
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Failed to create new process");
    }
    else if (pid == 0)
    {
        if (redirect_output(argv[argc - 1]) != 0) 
            return;
        
        argv[argc - 2] = NULL;
        if (execv(argv[0], argv))
            perror("Execution failed");
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}

void run_redirect_output(int argc, char **argv) {
    if (set_executable_path(argv) == 0)
        exec_redirect_output(argc, argv);
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

void run(char **argv)
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

void history()
{
    printf("%s", command_history);
}

void print_help()
{
    printf("These shell commands are defined internally.  Type `help' to see this list.\n");
    printf("quit\t\t\tExit the shell\n");
    printf("cd [dir]\t\tChange the shell working directory.\n");
    printf("cwd\t\t\tPrint the name of the current working directory.\n");
    printf("[program] [args ...]\tExecute program with absolute/relative path or found in PATH dirs with the given args.\n");
    printf("help\t\t\tDisplay information about builtin commands.\n");
    printf("history\t\t\tDisplay history of commands.\n");
}

void execute_command(int cmd_code, int argc, char **argv)
{
    if (cmd_code == 1) // help
        print_help();
    else if (cmd_code == 2) // cd command
        cd(argv);
    else if (cmd_code == 3) // cwd command
        cwd();
    else if (cmd_code == 4) // history command
        history();
    else if (cmd_code == 5) // redirect output
        run_redirect_output(argc, argv);
    else if (cmd_code == 6) // run command
        run(argv);
}

void add_command_to_history(char *input)
{
    strcat(command_history, input);
    strcat(command_history, "\n");
}

void destroy_executor()
{
    free(env_paths);
}