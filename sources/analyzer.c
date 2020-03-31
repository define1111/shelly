#include <stdio.h>
#include <stdlib.h>

#include "../include/parser.h"
#include "../include/analyzer.h"
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

    command->args = NULL;
    command->in = NULL;
    command->out = NULL;

    token_t *iter = NULL;
    unsigned int i;

    int flag_in = 1;
    int flag_out = 1;

    for (iter = conv[conv_number], i = 0; iter; iter = iter->next, ++i)
    {
        command->args = (char**) realloc(command->args, (i + 1) * sizeof(char*));
        if (command->args == NULL)
        {
            perror("realloc");
            exit(ALLOC_ERR);
        }
        
        if (iter->lex == LEX_ID || iter->lex == LEX_QUOTES)
            command->args[i] = iter->value;
        else if (iter->lex == LEX_MORE && flag_out)
        {
            flag_out = 0;
            if (iter->next != NULL && iter->next->lex == LEX_ID)
                command->out = iter->next->value;
            else
            {
                printf("syntax error: output file after > expected\n");
                free_conv(conv);
                free_command(command);
                return NULL;
            }
        }
        else if (iter->lex == LEX_LESS && flag_in)
        {
            flag_in = 0;
            if (iter->next != NULL && iter->next->lex == LEX_ID)
                command->in = iter->next->value;
            else
            {
                printf("syntax error: input file after < expected\n");
                free_conv(conv);
                free_command(command);
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
free_command(command_t *command)
{
    free(command->args);
    free(command);
}

void
free_commands(command_t **commands)
{
    unsigned int i;

    for (i = 0; commands[i] != NULL; ++i)
        free_command(commands[i]);
    
    free(commands);
}
