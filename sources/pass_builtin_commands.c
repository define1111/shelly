#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/pass_builtin_commands.h"
#include "../include/passes.h"
#include "../include/command.h"
#include "../include/conveyor.h"
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

void
run_builtin_commands(conveyor_t *conveyor, unsigned int num, token_t **tokens_conveyor)
{
    int save_stdout = -1;
    int save_stderr = -1;

    int i;
    int command_arg = 0;

    if (conveyor->commands[num]->fd_output_file != -1) 
    {
        save_stdout = dup(STDOUT_FILENO);
        dup2(conveyor->commands[num]->fd_output_file, STDOUT_FILENO);
        close(conveyor->commands[num]->fd_output_file);
    }
    if (conveyor->commands[num]->fd_error_output_file != -1)
    {
        save_stderr = dup(STDERR_FILENO);
        dup2(conveyor->commands[num]->fd_error_output_file, STDERR_FILENO);
        close(conveyor->commands[num]->fd_error_output_file);
    }

    switch (conveyor->commands[num]->builtin_command_type)
    {
    case BUILTIN_COMMAND_NONE:
        break;
    case BUILTIN_COMMAND_CD:
        /* add it after env */
        /*if (commands[0]->args[1] == NULL || strcmp(commands[0]->args[1], "~") == 0)
        {
            chdir(home);
        }*/

        /* temp */
        if (conveyor->commands[num]->args[1] == NULL)
            printf("cd: no args\n");
        if (chdir(conveyor->commands[num]->args[1]) == -1)
        {
            perror("chdir");
            exit(CHDIR_ERR);
        }
        break;
    case BUILTIN_COMMAND_MUR:   
        if (conveyor->commands[num]->args[1] != NULL && string_compare(conveyor->commands[num]->args[1], "-n") == 0)
        {
            if (conveyor->commands[num]->args[2] != NULL)
            {
                command_arg = atoi(conveyor->commands[num]->args[2]);
                for (i = 0; i < command_arg; ++i)
                    printf("mur ");
            }
        }
        printf(":3\n");
        exit(SUCCESS_EXIT);
    case BUILTIN_COMMAND_HELP:
        printf("Shelly, version 0.7.1\n");
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
        exit(SUCCESS_EXIT);
    case BUILTIN_COMMAND_EXIT:
        free_tokens_conveyor(tokens_conveyor);
        free_conveyor(conveyor);
        exit(SUCCESS_EXIT);
    }

    if (save_stdout != -1)
    {
        dup2(save_stdout, STDOUT_FILENO);
        close(save_stdout);
    }
    if (save_stderr != -1)
    {
        dup2(save_stderr, STDOUT_FILENO);
        close(save_stderr);
    }
}
