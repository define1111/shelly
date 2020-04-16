#ifndef SENTRY_BUILTIN_COMMANDS_H
#define SENTRY_BUILTIN_COMMANDS_H

#include "passes.h"
#include "analyzer.h"

pass_return_code_t run_builtin_commands(command_t **commands, token_t **conv, passes_t *current_pass);

#endif