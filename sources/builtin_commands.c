#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/builtin_commands.h"
#include "../include/passes.h"
#include "../include/analyzer.h"
#include "../include/error_list.h"

static int
string_length(const char *str)
{
    const char *ptr;

    for (ptr = str; *ptr; ++ptr)
        ;

    return ptr - str;
}

static int 
string_compare(const char *str1, const char *str2)
{
    for (; *str1 && *str1 == *str2; ++str1, ++str2)
        ;

    return (unsigned char) *str1 - (unsigned char) *str2;
}

/* only first command can be builtin. (maybe, I don't know) */
pass_return_code_t 
run_builtin_commands(command_t **commands, token_t **conv, passes_t *current_pass)
{
    int command_length = string_length(commands[0]->args[0]);

    if (command_length == 2)
    {
        if (string_compare(commands[0]->args[0], "cd") == 0)
        {
            /* add it after env */
            /*if (commands[0]->args[1] == NULL || strcmp(commands[0]->args[1], "~") == 0)
            {
                chdir(home);
            }*/

            /* temp */
            if (commands[0]->args[1] == NULL)
                printf("cd: no args\n");

            if (chdir(commands[0]->args[1]) == -1)
            {
                perror("chdir");
                exit(CHDIR_ERR);
            }

            *current_pass = PASS_FREE_ALLOCS - 1;
                
            return PASS_RET_CONTINUE;
        }
    }
    else if (command_length == 4)
    {
        if (string_compare(commands[0]->args[0], "help") == 0)
        {
            printf("Shelly, version 0.7\n");
            printf("This is my simple command line interpreter for Linux. Im writing it for self-education purposes.\n");
            printf("Builtin commands:\n");
            printf("cd\n");
            printf("help\n");
            printf("exit\n");
            printf("Support execute external commands with conveyor and redirect streams\n");

            *current_pass = PASS_FREE_ALLOCS - 1;

            return PASS_RET_CONTINUE;
        }
                
        if (string_compare(commands[0]->args[0], "exit") == 0)
        {
            free_conv(conv);
            free_commands(commands);
            return PASS_RET_SUCCESS;
        }
    }

    return PASS_RET_CONTINUE;
}