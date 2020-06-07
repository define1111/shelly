#ifndef _SENTRY_BUILTIN_COMMANDS_H
#define _SENTRY_BUILTIN_COMMANDS_H

#include "passes.h"
#include "command.h"
#include "conveyor.h"

builtin_command_type_t detect_buitin_command_type(command_t *command);
void run_builtin_commands(conveyor_t *conveyor, unsigned int num, token_t **tokens_conveyor);

#endif
