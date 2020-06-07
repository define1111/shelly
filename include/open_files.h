#ifndef _SENTRY_OPEN_FILES_H
#define _SENTRY_OPEN_FILES_H

#include "passes.h"
#include "conveyor.h"

pass_return_code_t run_open_files(conveyor_t *conveyor, token_t **tokens_conveyor);

#endif
