#ifndef SENTRY_PASS_OPEN_FILES_H
#define SENTRY_PASS_OPEN_FILES_H

#include "passes.h"
#include "command.h"

pass_return_code_t run_open_files(command_t **commands, token_t **conv, unsigned int conveyor_length);

#endif
