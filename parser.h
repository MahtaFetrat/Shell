extern const int MAX_CMD_ARG_COUNT;
extern const int MAX_CMD_ARG_LEN;

/**
 * Compiles all the pre-defined command patterns.
 */
void initialize_parser();

/**
 * Parses the user input string with a matching command
 *
 * @param char *input ... user input command
 * @param char **cmd_args ... placeholder array of the parsed arguments
 *
 * @return code of the matching command or -1 if no match found
 */
int parse(char *input, char **cmd_args);