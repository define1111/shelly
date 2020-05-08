#ifndef SENTRY_PASS_BUILTIN_COMMANDS_H
#define SENTRY_PASS_BUILTIN_COMMANDS_H

#include "passes.h"
#include "command.h"

builtin_command_type_t detect_buitin_command_type(command_t *command);
pass_return_code_t run_builtin_commands(command_t **commands, token_t **conv, passes_t *current_pass);

#endif
