#include <stdio.h>
#include <stdlib.h>

#include "../include/parser.h"
#include "../include/command.h"
#include "../include/conveyor.h"
#include "../include/error_list.h"

command_t *
get_command(token_t **tokens_conveyor, unsigned int num)
{
    token_t *iter = NULL;
    unsigned int i = 0;

    command_t *command = (command_t*) malloc(sizeof(command_t));
    if (command == NULL)
    {
        perror("malloc");
        exit(ALLOC_ERR);
    }

    command->builtin_command_type = BUILTIN_COMMAND_NONE;
    command->pid = -1;
    command->args = NULL;
    command->input_file = NULL;
    command->output_file = NULL;
    command->error_output_file = NULL;
    command->fd_input_file = -1;
    command->fd_output_file = -1;
    command->fd_error_output_file = -1;
    command->output_type = OUTPUT_TYPE_NONE;
    command->error_output_type = OUTPUT_TYPE_NONE;

    for (iter = tokens_conveyor[num]; iter; iter = iter->next)
    {
        if (iter->lex == LEX_ID || iter->lex == LEX_SINGLE_QUOTES || iter->lex == LEX_DOUBLE_QUOTES)
        {
            command->args = (char**) realloc(command->args, (i + 1) * sizeof(char*));
            if (command->args == NULL)
            {
                perror("realloc");
                exit(ALLOC_ERR);
            }
            command->args[i++] = iter->value;
        }
        else if (iter->lex == LEX_MORE)
        {
            if (iter->next != NULL && iter->next->lex == LEX_ID)
            {
                command->output_file = iter->next->value;
                command->output_type = OUTPUT_TYPE_REWRITE;
                iter = iter->next;
            }
            else
            {
                printf("syntax error: output file after > expected\n");
                free_tokens_conveyor(tokens_conveyor);
                free(command->args);
                free(command);
                return NULL;
            }
        }
        else if (iter->lex == LEX_MOREMORE)
        {
            if (iter->next != NULL && iter->next->lex == LEX_ID)
            {
                command->output_file = iter->next->value;
                command->output_type = OUTPUT_TYPE_APPEND;
                iter = iter->next;
            }
            else
            {
                printf("syntax error: output file after >> expected\n");
                free_tokens_conveyor(tokens_conveyor);
                free(command->args);
                free(command);
                return NULL;
            }
        }
        else if (iter->lex == LEX_LESS)
        {
            if (iter->next != NULL && iter->next->lex == LEX_ID)
            {
                command->input_file = iter->next->value;
                iter = iter->next;
            }
            else
            {
                printf("syntax error: input file after < expected\n");
                free_tokens_conveyor(tokens_conveyor);
                free(command->args);
                free(command);
                return NULL;
            }
        }
        else if (iter->lex == LEX_TWO_MORE)
        {
            if (iter->next != NULL && iter->next->lex == LEX_ID)
            {
                command->error_output_file = iter->next->value;
                command->error_output_type = OUTPUT_TYPE_REWRITE;
                iter = iter->next;
            }
            else
            {
                printf("syntax error: output file after 2> expected\n");
                free_tokens_conveyor(tokens_conveyor);
                free(command->args);
                free(command);
                return NULL;
            }
        }
        else if (iter->lex == LEX_TWO_MOREMORE)
        {
            if (iter->next != NULL && iter->next->lex == LEX_ID)
            {
                command->error_output_file = iter->next->value;
                command->error_output_type = OUTPUT_TYPE_APPEND;
                iter = iter->next;
            }
            else
            {
                printf("syntax error: output file after 2>> expected\n");
                free_tokens_conveyor(tokens_conveyor);
                free(command->args);
                free(command);
                return NULL;
            }
        }
    } /* end for */

    command->args = (char**) realloc(command->args, (i + 1) * sizeof(char*));
    if (command->args == NULL)
    {
        perror("realloc");
        exit(ALLOC_ERR);
    }
    command->args[i] = NULL;
    
    return command;
}

void
free_commands(command_t **commands)
{
    unsigned int i;

    for (i = 0; commands[i] != NULL; ++i)
    {
        free(commands[i]->args);
        free(commands[i]);
    }
    
    free(commands);
}
