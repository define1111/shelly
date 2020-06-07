#ifndef _SENTRY_COMMAND_H
#define _SENTRY_COMMAND_H

#include <sys/types.h>
#include "parser.h"

enum builtin_command_type_t
{
    BUILTIN_COMMAND_NONE = 0,
    BUILTIN_COMMAND_CD,
    BUILTIN_COMMAND_MUR,
    BUILTIN_COMMAND_HELP,
    BUILTIN_COMMAND_EXIT
};
typedef enum builtin_command_type_t builtin_command_type_t;

enum output_type_t { OUTPUT_TYPE_NONE = 0, OUTPUT_TYPE_REWRITE, OUTPUT_TYPE_APPEND };
typedef enum output_type_t output_type_t;

struct command_t
{
    builtin_command_type_t builtin_command_type;
    pid_t pid;
    char **args;
    char *input_file;
    char *output_file;
    char *error_output_file;
    int fd_input_file;
    int fd_output_file;
    int fd_error_output_file;
    output_type_t output_type;
    output_type_t error_output_type;
};
typedef struct command_t command_t; 

command_t *get_command(token_t **tokens_conveyor, unsigned int num);
void free_commands(command_t **commands);

#endif
