/**
 * This module executes the user input commands.
*/

/**
 * Sets environmental execution paths.
 */
void initialize_executor();

/**
 * Executes the parsed input command
 *
 * @param int cmd_code ... code of the command to be executed
 * @param int argc ... count of arguments of the command
 * @param char **argv ... array of arguments of the command
 * 
 */
void execute_command(int cmd_code, int argc, char **argv);

/**
 * Frees executor allocated spaces.
 */
void destroy_executor();

/**
 * Add the input command to command history.
 */
void add_command_to_history(char *input);
