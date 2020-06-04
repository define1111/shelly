#ifndef SENTRY_PASS_OPEN_FILES_H
#define SENTRY_PASS_OPEN_FILES_H

#include "passes.h"
#include "conveyor.h"

pass_return_code_t run_open_files(conveyor_t *conveyor, token_t **tokens_conveyor);

#endif
