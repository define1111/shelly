#include <stdio.h>
#include <stdlib.h>

#include "../include/conveyor.h"
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
                free_tokens_conveyor(conv);
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
get_conveyor_length_from_tokens(token_t **tokens_conveyor)
{
    unsigned int i;

    for (i = 0; tokens_conveyor[i] != NULL; ++i)
        ;
    
    return i;
}

void
free_tokens_conveyor(token_t **tokens_conveyor)
{
    unsigned int i;
    
    for (i = 0; tokens_conveyor[i] != NULL; ++i)
        free_token_list(&tokens_conveyor[i]);

    free(tokens_conveyor);
}
