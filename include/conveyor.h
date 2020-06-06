#ifndef SENTRY_CONVEYOR_H
#define SENTRY_CONVEYOR_H

#include "command.h"

struct conveyor_t
{
    command_t **commands; /* array of commands */
    unsigned int length; /* count of commands */
};
typedef struct conveyor_t conveyor_t;

token_t **conv_parse(token_t *token_list_head);
void free_tokens_conveyor(token_t **tokens_conveyor);
unsigned int get_conveyor_length_from_tokens(token_t **tokens_conveyor);

#endif
