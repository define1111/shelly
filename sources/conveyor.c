#include <stdio.h>
#include <stdlib.h>

#include "../include/conveyor.h"
#include "../include/parser.h"
#include "../include/command.h"
#include "../include/error_list.h"

token_t **
tokens_conveyor_parse(token_t *token_list_head)
{
    token_t *iter = NULL;
    token_t **tokens_conveyor = NULL; 
    unsigned int i = 1;

    tokens_conveyor = (token_t**) malloc(sizeof(token_t*));
    if (tokens_conveyor == NULL)
    {
        perror("malloc");
        exit(ALLOC_ERR);
    }

    tokens_conveyor[0] = token_list_head;

    for (iter = token_list_head; iter; iter = iter->next)
    {
        if (iter->lex == LEX_CONV)
        {
            tokens_conveyor = (token_t**) realloc(tokens_conveyor, ++i * sizeof(token_t*));
            if (tokens_conveyor == NULL)
            {
                perror("realloc");
                exit(ALLOC_ERR);
            }

            if (iter->next == NULL || iter->next->lex != LEX_ID)
            {
                printf("syntax error: command after | expected\n");
                tokens_conveyor[i - 1] = NULL;
                free_tokens_conveyor(tokens_conveyor);
                return NULL;
            }

            iter->prev->next = NULL;
            tokens_conveyor[i - 1] = iter->next;
            iter->next->prev = NULL;
            free(iter);
            iter = tokens_conveyor[i - 1];
        }
    }

    tokens_conveyor = (token_t**) realloc(tokens_conveyor, ++i * sizeof(token_t*));
    if (tokens_conveyor == NULL)
    {
        perror("realloc");
        exit(ALLOC_ERR);
    }
    tokens_conveyor[i - 1] = NULL;

    return tokens_conveyor;
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

pass_return_code_t
run_conveyor(conveyor_t *conveyor, token_t **tokens_conveyor)
{

}
