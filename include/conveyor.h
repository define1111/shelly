#ifndef SENTRY_CONVEYOR_H
#define SENTRY_CONVEYOR_H

#include "command.h"
#include "passes.h"

struct conveyor_t
{
    command_t **commands; /* array of commands */
    unsigned int length; /* count of commands */
    int (*pipe_fd)[2]; /* array of pointers to pipe_fd[2] */
};
typedef struct conveyor_t conveyor_t;

token_t **tokens_conveyor_parse(token_t *token_list_head);
void free_tokens_conveyor(token_t **tokens_conveyor);
unsigned int get_conveyor_length_from_tokens(token_t **tokens_conveyor);
void run_conveyor(conveyor_t *conveyor, token_t **tokens_conveyor);
void free_conveyor(conveyor_t *conveyor);

#endif
