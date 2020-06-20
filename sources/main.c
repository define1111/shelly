#include <stdio.h>

#include "../include/passes.h"
#include "../include/error_list.h"

void promt(void);

int
main()
{
    pass_return_code_t pass_code = PASS_RET_SUCCESS;

    while (1) /* MAIN LOOP */
    {
        promt();
        
        pass_code = run_passes();
        if (pass_code == PASS_RET_SUCCESS) break;

    } /* while (1) */

    return SUCCESS_EXIT;
}

void
promt()
{
    putchar('$');
    putchar(' ');
}
