#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/pass_builtin_commands.h"
#include "../include/passes.h"
#include "../include/command.h"
#include "../include/error_list.h"
#include "../include/shelly_string.h"

builtin_command_type_t 
detect_buitin_command_type(command_t *command)
{
    int command_length = string_length(command->args[0]);
    builtin_command_type_t builtin_command_type = BUILTIN_COMMAND_NONE;

    switch (command_length)
    {
    case 2:
        if (string_compare(command->args[0], "cd") == 0)
            builtin_command_type = BUILTIN_COMMAND_CD;
        break;
    case 3:
        if (string_compare(command->args[0], "mur") == 0)
            builtin_command_type = BUILTIN_COMMAND_MUR;
        break;
    case 4:
        if (string_compare(command->args[0], "help") == 0)
        {
            builtin_command_type = BUILTIN_COMMAND_HELP;
            break;
        }
        if (string_compare(command->args[0], "exit") == 0)
        {
            builtin_command_type = BUILTIN_COMMAND_EXIT;
            break;
        }
        break;
    default:
        break;
    }

    return builtin_command_type;
}

pass_return_code_t 
run_builtin_commands(command_t **commands, token_t **conv, passes_t *current_pass)
{
    switch (commands[0]->builtin_command_type)
    {
    case BUILTIN_COMMAND_NONE:
        return PASS_RET_CONTINUE;
    case BUILTIN_COMMAND_CD:
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
    case BUILTIN_COMMAND_MUR:   
        if (commands[0]->args[1] != NULL && string_compare(commands[0]->args[1], "-n") == 0)
        {
            if (commands[0]->args[2] != NULL)
            {
                int num = atoi(commands[0]->args[2]);
                for (int i = 0; i < num; ++i)
                    printf("mur ");
            }
        }
        printf(":3\n");
        *current_pass = PASS_FREE_ALLOCS - 1;
        return PASS_RET_CONTINUE;
    case BUILTIN_COMMAND_HELP:
        printf("Shelly, version 0.7\n");
        printf("This is my simple command line interpreter for Linux. Im writing it for self-education purposes.\n");
        printf("Builtin commands:\n");
        printf("cd\n");
        printf("mur\n");
        printf("help\n");
        printf("exit\n");
        printf("command mur makes the shelly purr\n");
        printf("$ mur -n 3\n");
        printf("mur mur mur :3\n");
        printf("Support execute external commands with conveyor and redirect streams\n");
        *current_pass = PASS_FREE_ALLOCS - 1;
        return PASS_RET_CONTINUE;
    case BUILTIN_COMMAND_EXIT:
        free_conv(conv);
        free_commands(commands);
        return PASS_RET_SUCCESS;
    }
}