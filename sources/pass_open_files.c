#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../include/pass_open_files.h"
#include "../include/conveyor.h"

pass_return_code_t
run_open_files(command_t **commands, token_t **conv, unsigned int conveyor_length)
{
    unsigned int j;

    if (commands[0]->input_file != NULL)
    {
        commands[0]->fd_input_file = open(commands[0]->input_file, O_RDONLY, S_IRUSR|S_IWUSR);
        if (commands[0]->fd_input_file == -1)
        {
            printf("open error: can't open file %s\n", commands[0]->input_file);
            free_conv(conv);
            free_commands(commands);
            return PASS_RET_CONTINUE;
        }
    }
    for (j = 0; j < conveyor_length; ++j)
    {
        if (commands[j]->error_output_type == OUTPUT_TYPE_REWRITE)
        {
            commands[j]->fd_error_output_file = open(commands[j]->error_output_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
            if (commands[j]->fd_error_output_file == -1)
            {
                printf("open error: can't open file %s\n", commands[j]->error_output_file);
                free_conv(conv);
                free_commands(commands);
                return PASS_RET_CONTINUE;
            }
        }
        else if (commands[j]->error_output_type == OUTPUT_TYPE_APPEND)
        {
            commands[j]->fd_error_output_file = open(commands[j]->error_output_file, O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);
            if (commands[j]->fd_error_output_file == -1)
            {
                printf("open error: can't open file %s\n", commands[j]->error_output_file);
                free_conv(conv);
                free_commands(commands);
                return PASS_RET_CONTINUE;
            }
        }
    }
    if (commands[conveyor_length - 1]->output_type == OUTPUT_TYPE_REWRITE)
    {
        commands[conveyor_length - 1]->fd_output_file = open(commands[conveyor_length - 1]->output_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
        if (commands[conveyor_length - 1]->fd_output_file == -1)
        {
            printf("open error: can't open file %s\n", commands[conveyor_length - 1]->output_file);
            free_conv(conv);
            free_commands(commands);
            return PASS_RET_CONTINUE;
        }
    }
    else if (commands[conveyor_length - 1]->output_type == OUTPUT_TYPE_APPEND)
    {
        commands[conveyor_length - 1]->fd_output_file = open(commands[conveyor_length - 1]->output_file, O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);
        if (commands[conveyor_length - 1]->fd_output_file == -1)
        {
            printf("open error: can't open file %s\n", commands[conveyor_length - 1]->output_file);
            free_conv(conv);
            free_commands(commands);
            return PASS_RET_CONTINUE;
        }
    }

    return PASS_RET_SUCCESS;
}
