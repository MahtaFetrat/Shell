#include "executor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_ENV_PATHS 50
#define MAX_PATH_LEN 200
#define MAX_COMMANDS_LEN 1000
#define TEMPFILENAME "/tmp/shellt"

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
void replace_home(char *path)
{
    if (strcmp(path, "~") == 0 || (path[0] == '~' && path[1] == '/'))
    {
        char home_replaced[200];
        sprintf(home_replaced, "%s%s", getenv("HOME"), path + 1);
        strcpy(path, home_replaced);
    }
}

/**
 * Converts any user input path for the program to some absolute executable path
 */
int set_executable_path(char *path)
{
    replace_home(path);
    if (strchr(path, '/') != NULL) // Absolute or Relative path
    {
        if (access(path, F_OK) != 0)
        {
            perror("Exection failed: No such file or directory\n");
            return 1;
        }
        else if (access(path, X_OK) != 0)
        {
            perror("Exection failed: Permission denied\n");
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
            sprintf(catenated_path, "%s/%s", env_paths[i], path);
            if (access(catenated_path, X_OK) == 0)
            {
                strcpy(path, catenated_path);
                return 0;
            }
            i++;
        }
        // Neither a path nor a file found in $PATH
        fprintf(stderr, "%s: Invalid use of command\n", path);
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

void run(char **argv)
{
    if (set_executable_path(argv[0]) == 0)
        exec(argv);
}

int redirect_input(char *filepath)
{
    replace_home(filepath);
    int file = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

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
    if (set_executable_path(argv[0]) == 0)
        exec_redirect_input(argc, argv);
}

int redirect_output(char *filepath)
{
    replace_home(filepath);
    int file = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

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
    if (set_executable_path(argv[0]) == 0)
        exec_redirect_output(argc, argv);
}

int redirect_pipe_out(int tempfile)
{
    if (dup2(tempfile, STDOUT_FILENO) < 0 || dup2(tempfile, STDERR_FILENO) < 0) // Redirect STDOUT and STDERR to file.
    {
        perror("Couldn't pipe output\n");
        return 1;
    }

    return 0;
}

void exec_pipe_out(char **argv, int tempfile)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Failed to create new process");
    }
    else if (pid == 0)
    {
        if (redirect_pipe_out(tempfile) != 0)
            return;

        if (execv(argv[0], argv))
            perror("Execution failed");
    }
}

int create_tempfile() {
    int tempfile = open(TEMPFILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (tempfile < 0)
    {
        perror("Couldn't open pipe file\n");
        return -1;
    }

    return tempfile;
}

/**
 * utility function to find index of | char in the arguments.
*/
int get_pipe_char_idx(int argc ,char **argv)
{
    for (int i = 0; i< argc; i++)
        if (strcmp(argv[i], "|") == 0)
            return i;
}

/**
 * utility function to shift the arguments of the second command to the first.
*/
void shift_args(char **argv, int argc, int pipe_char_idx)
{
    strcpy(argv[0], argv[pipe_char_idx + 1]);
    for (int i = 1; i < argc - pipe_char_idx - 1; i++)
    {
        argv[i] = argv[i + pipe_char_idx + 1];
    }
    argv[argc - pipe_char_idx - 1] = TEMPFILENAME;
    argv[argc - pipe_char_idx] = NULL;
}

void run_piped(int argc, char **argv)
{
    // Replace | with NULL which separates two commands.
    int pipe_char_idx = get_pipe_char_idx(argc, argv);
    argv[pipe_char_idx] = NULL;

    if (set_executable_path(argv[0]) != 0)
        return;
    
    int tempfile = create_tempfile();
    if (tempfile < 0)
        return;
    exec_pipe_out(argv, tempfile);
    close(tempfile);

    shift_args(argv, argc, pipe_char_idx);
    run(argv);
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
    if (set_executable_path(argv[0]) == 0)
        exec_background(argc, argv);
}

void cwd()
{
    char cwd[200];
    getcwd(cwd, 200);
    printf("%s\n", cwd);
}

void cd(char **argv)
{
    replace_home(argv[1]);

    if (chdir(argv[1]) != 0)
    {
        perror("cd: Inaccessible directory\n");
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
    else if (cmd_code == 6) // redirect input
        run_redirect_input(argc, argv);
    else if (cmd_code == 7) // run piped
        run_piped(argc, argv);
    else if (cmd_code == 8) // run background
        run_background(argc, argv);
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
    if (access(TEMPFILENAME, F_OK))
        remove(TEMPFILENAME);
}