#ifndef SENTRY_PASSES_H
#define SENTRY_PASSES_H

enum passes 
{ 
    PASS_TOKENIZATION = 0, /* start pass */
    PASS_SPLIT_TOKENS_CONVEYOR,
    PASS_INIT_CONVEYOR,
    PASS_GET_COMMANDS_FROM_TOKENS_CONVEYOR,
    PASS_OPEN_FILES_FOR_CONVEYOR,  
    PASS_EXECUTE_BUILTIN_COMMAND, 
    PASS_EXECUTE_EXTERNAL_COMMAND,
    PASS_CLOSE_FILES_FOR_CONVEYOR,
    PASS_FREE_ALLOCS,
    PASS_END
};
typedef enum passes passes_t;

enum pass_return_code { PASS_RET_SUCCESS = 0, PASS_RET_CONTINUE, PASS_RET_ERR };
typedef enum pass_return_code pass_return_code_t;

pass_return_code_t run_passes(void);

#endif
