#include <stdio.h>
#include <stdlib.h>

#include "../include/parser.h"
#include "../include/command.h"
#include "../include/error_list.h"

token_t **
conv_parse(token_t *token_list_head)
{
    token_t *iter = NULL;
    token_t **conv = NULL; 
    unsigned int i = 1;

    conv = (token_t**) malloc(sizeof(token_t*));
    if (conv == NULL)
    {
        perror("malloc");
        exit(ALLOC_ERR);
    }

    conv[0] = token_list_head;

    for (iter = token_list_head; iter; iter = iter->next)
    {
        if (iter->lex == LEX_CONV)
        {
            conv = (token_t**) realloc(conv, ++i * sizeof(token_t*));
            if (conv == NULL)
            {
                perror("realloc");
                exit(ALLOC_ERR);
            }

            if (iter->next == NULL || iter->next->lex != LEX_ID)
            {
                printf("syntax error: command after | expected\n");
                conv[i - 1] = NULL;
                free_conv(conv);
                return NULL;
            }

            iter->prev->next = NULL;
            conv[i - 1] = iter->next;
            iter->next->prev = NULL;
            free(iter);
            iter = conv[i - 1];
        }
    }

    conv = (token_t**) realloc(conv, ++i * sizeof(token_t*));
    if (conv == NULL)
    {
        perror("realloc");
        exit(ALLOC_ERR);
    }
    conv[i - 1] = NULL;

    return conv;
}

unsigned int
conv_len(token_t **conv)
{
    unsigned int i;

    for (i = 0; conv[i] != NULL; ++i)
        ;
    
    return i;
}

void
free_conv(token_t **conv)
{
    unsigned int i;
    
    for (i = 0; conv[i] != NULL; ++i)
        free_token_list(&conv[i]);

    free(conv);
}

command_t *
get_command(token_t **conv, unsigned int conv_number)
{
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

    token_t *iter = NULL;
    unsigned int i = 0;

    for (iter = conv[conv_number]; iter; iter = iter->next)
    {
        if (iter->lex == LEX_ID || iter->lex == LES_SINGLE_QUOTES || iter->lex == LEX_DOUBLE_QUOTES)
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
                free_conv(conv);
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
                free_conv(conv);
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
                free_conv(conv);
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
                free_conv(conv);
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
                free_conv(conv);
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
