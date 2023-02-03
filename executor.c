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
 * Replace ~ by HOME since non-recognized by chdir.
 */
void replace_home(char *new_path, char *path)
{
    strcpy(new_path, path);
    if (strcmp(path, "~") == 0 || (path[0] == '~' && path[1] == '/'))
    {
        sprintf(new_path, "%s%s", getenv("HOME"), path + 1);
    }
}

/**
 * Converts any user input path for the program to some absolute executable path
 */
char *set_executable_path(char *path)
{
    char *executable_path = (char *) malloc (200 * sizeof(char));

    replace_home(executable_path, path);

    if (strchr(executable_path, '/') != NULL) // Absolute or Relative path
    {
        if (access(executable_path, F_OK) != 0)
        {
            perror("Exection failed: No such file or directory\n");
            return NULL;
        }
        else if (access(executable_path, X_OK) != 0)
        {
            perror("Exection failed: Permission denied\n");
            return NULL;
        }
        return executable_path;
    }
    else // Lookup in the $PATH directories
    {
        int i = 0;
        char catenated_path[200];

        while (env_paths[i] != NULL)
        {
            sprintf(catenated_path, "%s/%s", env_paths[i], executable_path);
            if (access(catenated_path, X_OK) == 0)
            {
                strcpy(executable_path, catenated_path);
                return executable_path;
            }
            i++;
        }
        // Neither a path nor a file found in $PATH
        fprintf(stderr, "%s: Invalid use of command\n", path);
        return NULL;
    }
}

int redirect_input(char *filepath)
{
    char *abs_filepath = (char *) malloc (200 * sizeof(char));

    replace_home(abs_filepath, filepath);

    int file = open(abs_filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (file < 0)
    {
        perror("Couldn't open input file\n");
        return 1;
    }

    if (dup2(file, STDIN_FILENO) < 0) // Redirect STDIN.
    {
        perror("Couldn't redirect input from file\n");
        return 2;
    }

    close(file);
    return 0;
}

void exec_redirect_input(int argc, char **argv)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Failed to create new process");
    }
    else if (pid == 0)
    {
        if (redirect_input(argv[argc - 1]) != 0)
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

void run_redirect_input(int argc, char **argv)
{
    char *executable_path = set_executable_path(argv[0]);
    if (executable_path == NULL)
        return;
    argv[0] = executable_path;
    exec_redirect_input(argc, argv);
}

int redirect_output(char *filepath)
{
    char *abs_filepath = (char *) malloc (200 * sizeof(char));

    replace_home(abs_filepath, filepath);
    int file = open(abs_filepath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (file < 0)
    {
        perror("Couldn't open output file\n");
        return 1;
    }

    if (dup2(file, STDOUT_FILENO) < 0 || dup2(file, STDERR_FILENO) < 0) // Redirect STDOUT and STDERR to file.
    {
        perror("Couldn't redirect output to file\n");
        return 2;
    }

    close(file);
    return 0;
}

void exec_redirect_output(int argc, char **argv)
{
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

void run_redirect_output(int argc, char **argv)
{
    char *executable_path = set_executable_path(argv[0]);
    if (executable_path == NULL)
        return;
    argv[0] = executable_path;
    exec_redirect_output(argc, argv);
}

void exec_background(int argc, char **argv)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Failed to create new process");
    }
    else if (pid == 0)
    {
        argv[argc - 1] = NULL;
        if (execv(argv[0], argv))
            perror("Execution failed");
    }
}

void run_background(int argc, char **argv)
{
    char *executable_path = set_executable_path(argv[0]);
    if (executable_path == NULL)
        return;
    argv[0] = executable_path;
    exec_background(argc, argv);
}

void run_exec_parallel(char *command)
{
    char **argv = (char **)malloc(50 * sizeof(char *));
    argv[0] = (char *)malloc(100 * sizeof(char *));

    char *ptr;
    strcpy(argv[0], strtok_r(command, " ", &ptr));

    int j = 0;
    while (argv[j] != NULL)
    {
        argv[++j] = strtok_r(NULL, " ", &ptr);
    }
    if (set_executable_path(argv[0]) == 0)
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
    }
}

void run_parallel(int argc, char **argv)
{
    char input_buffer[50 * (100 + 2)];
    for (int i = 0; i < argc; i++)
    {
        strcat(input_buffer, " ");
        strcat(input_buffer, argv[i]);
    }

    char **commands = (char **)malloc(50 * sizeof(char *));
    for (int i = 0; i < 50; i++)
    {
        commands[i] = (char *)malloc(30 * sizeof(char));
    }

    int count;
    char *saveptr;
    strcpy(commands[0], strtok_r(input_buffer, ";", &saveptr));

    int i = 0;
    while (commands[i] != NULL)
    {
        commands[++i] = strtok_r(NULL, ";", &saveptr);
    }
    count = i;

    for (int i = 0; i < count; i++)
    {
        run_exec_parallel(commands[i]);
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

void run(char **argv)
{
    char *executable_path = set_executable_path(argv[0]);
    if (executable_path == NULL)
        return;
    argv[0] = executable_path;
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
    char *abs_path = (char *) malloc (200 * sizeof(char));

    replace_home(abs_path, argv[1]);

    if (chdir(abs_path) != 0)
    {
        perror("cd: Inaccessible directory\n");
    }

    free(abs_path);
}

void history()
{
    printf("%s", command_history);
}

void print_help()
{
    printf("These shell commands are defined internally.  Type `help' to see this list.\n");
    printf("quit\t\t\t\t\t\tExit the shell\n");
    printf("cd [dir]\t\t\t\t\tChange the shell working directory.\n");
    printf("cwd\t\t\t\t\t\tPrint the name of the current working directory.\n");
    printf("[program] [args ...]\t\t\t\tExecute program with absolute/relative path or found in PATH dirs with the given args.\n");
    printf("[program] [args ...] &\t\t\t\tExecute program in background.\n");
    printf("[program] [args ...] | [program] [args ...]\tExecutes second program with the first program output.\n");
    printf("[program] [args ...] > [file]\t\t\tExecutes the program and writes its output to file.\n");
    printf("[program] [args ...] < [file]\t\t\tExecutes the program with the file as its input stream.\n");
    printf("help\t\t\t\t\t\tDisplay information about builtin commands.\n");
    printf("history\t\t\t\t\t\tDisplay history of commands.\n");
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
    else if (cmd_code == 6) // redirect input
        run_redirect_input(argc, argv);
    else if (cmd_code == 7) // run background
        run_background(argc, argv);
    else if (cmd_code == 8)
        run_parallel(argc, argv);
    else if (cmd_code == 9) // run command
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