#ifndef SENTRY_CONVEYOR_H
#define SENTRY_CONVEYOR_H

#include "command.h"

struct conveyor_t
{
    command_t *commands;
    unsigned int length;
};

token_t **conv_parse(token_t *token_list_head);
void free_conv(token_t **conv);
unsigned int conv_len(token_t **conv);

#endif
