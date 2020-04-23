#ifndef SENTRY_ANALYZER_H
#define SENTRY_ANALYZER_H

#include "parser.h"

enum output_type_t { OUTPUT_TYPE_NONE = 0, OUTPUT_TYPE_REWRITE, OUTPUT_TYPE_APPEND };
typedef enum output_type_t output_type_t;

struct command_t
{
    char **args;
    char *in;
    char *out;
    char *err_out;
    output_type_t output_type;
};
typedef struct command_t command_t; 

token_t **conv_parse(token_t *token_list_head);
void free_conv(token_t **conv);

command_t *get_command(token_t **conv, unsigned int conv_number);
void free_command(command_t *command);
void free_commands(command_t **commands);

unsigned int conv_len(token_t **conv);

#endif
