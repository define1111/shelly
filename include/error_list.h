#ifndef _SENTRY_ERROR_LIST
#define _SENTRY_ERROR_LIST

enum error_list { SUCCESS_EXIT = 0, ALLOC_ERR, OPEN_FILE_ERR, 
                  FORK_ERR, EXEC_ERR, PIPE_ERR, CHDIR_ERR };

#endif
