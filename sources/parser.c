#include <stdio.h> 
#include <stdlib.h>

#include "../include/parser.h"
#include "../include/error_list.h"
#include "../include/shelly_string.h"

static token_t *
get_tail(token_t *head)
{
    for (; head->next; head = head->next)
        ;
    return head;
}

static token_t *
push_tail_token(token_t *head, lex_t lex, char *value)
{
    token_t *old_tail = NULL;
    token_t *new_tail = (token_t *) malloc(sizeof(token_t));
    if (new_tail == NULL)
    {
        perror("malloc");
        exit(ALLOC_ERR);
    }

    new_tail->lex = lex;
    new_tail->value = value;
    new_tail->next = NULL;

    if (head == NULL)
    {
        new_tail->prev = NULL;
        return new_tail;
    }

    old_tail = get_tail(head);
    old_tail->next = new_tail;
    new_tail->prev = old_tail;

    return head;
}

token_t *
parse_step_1()
{
    int ch = getchar();
    state_lex_t state = STATE_LOOP;
    char *value = NULL;
    int i = 0;
    token_t *head = NULL;
    int is_read = 0;

    if (ch == EOF)
    {
        putchar('\n');
        exit(SUCCESS_EXIT);
    }

    while (1)
    {
        if (is_read) ch = getchar();

        switch (state)
        {
        case STATE_LOOP:
            if (ch == ' ' || ch == '\t')
            {
                state = STATE_LOOP;
                is_read = 1;
            }
            else if (ch == EOF || ch == '\n')
            {
                state = STATE_END;
                is_read = 0;
            }
            else if (ch == '"')
            {
                state = STATE_DOUBLE_QUOTES;
                is_read = 1;
            }
            else if (ch == '\'')
            {
                state = STATE_SINGLE_QUOTES;
                is_read = 1;
            }
            else if (ch == '>')
            {
                state = STATE_LOOP;
                is_read = 1;
                head = push_tail_token(head, LEX_MORE, NULL); 
            }
            else if (ch == '<')
            {
                state = STATE_LOOP;
                is_read = 1;
                head = push_tail_token(head, LEX_LESS, NULL);
            }
            else if (ch == '&')
            {
                state = STATE_LOOP;
                is_read = 1;
                head = push_tail_token(head, LEX_AND, NULL);
            }
            else if (ch == '|')
            {
                state = STATE_LOOP;
                is_read = 1;
                head = push_tail_token(head, LEX_CONV, NULL);
            }
            else if (ch == ';')
            {
                state = STATE_LOOP;
                is_read = 1;
                head = push_tail_token(head, LEX_SEMICOLON, NULL);
            }
            else if (ch == '\\')
            {
                state = STATE_LOOP;
                is_read = 1;
                head = push_tail_token(head, LEX_BACKSLAH, NULL);
            }
            else if (ch == '#')
            {
                state = STATE_LOOP;
                is_read = 1;
                head = push_tail_token(head, LEX_SHARP, NULL);
            }
            else
            {
                state = STATE_IN_ID;
                is_read = 0;
            }
            break;
        case STATE_IN_ID:
            value = (char*) realloc(value, ++i * sizeof(char));
            if (value == NULL)
            {
                perror("realloc");
                exit(ALLOC_ERR);
            }
            if (!(ch == ' ' || ch == '\t' || ch == EOF || ch == '\n' || ch == '"' || \
                  ch == '\'' || ch == '>' || ch == '<' || ch == '&' || ch == '|' || ch == ';' || \
                  ch == '#'))
            {
                state = STATE_IN_ID;
                if (ch == '\\') /* mb another one state of DFA for spaces and end token? */
                {
                    ch = getchar();
                    if (ch == ' ')
                    {
                        is_read = 1;
                        value[i - 1] = (char) ch;
                        break;
                    }
                    else if (ch == EOF || ch == '\n' || ch == '"')
                    {
                        state = STATE_LOOP;
                        is_read = 0;
                        value[i - 1] = '\0';
                        head = push_tail_token(head, LEX_ID, value);
                        value = NULL;
                        i = 0;
                        break;
                    }
                }
                is_read = 1;
                value[i - 1] = (char) ch;
            }
            if (ch == ' ' || ch == '\t' || ch == EOF || ch == '\n' || ch == '"' || \
                ch == '\'' ||ch == '>' || ch == '<' || ch == '&' || ch == '|' || ch == ';' || \
                ch == '#')
            {
                state = STATE_LOOP;
                is_read = 0;
                value[i - 1] = '\0';
                head = push_tail_token(head, LEX_ID, value);
                value = NULL;
                i = 0;
            }
            break;
        case STATE_DOUBLE_QUOTES:
            value = (char*) realloc(value, ++i * sizeof(char));
            if (value == NULL)
            {
                perror("realloc");
                exit(ALLOC_ERR);
            }
            if (ch != '"')
            {
                state = STATE_DOUBLE_QUOTES;
                is_read = 1;
                value[i - 1] = (char) ch;
            }
            if (ch == '"')
            {
                state = STATE_LOOP;
                is_read = 1;
                value[i - 1] = '\0';
                head = push_tail_token(head, LEX_DOUBLE_QUOTES, value);
                value = NULL;
                i = 0;
            }
            if (ch == EOF || ch == '\n') /* mb another state for error ? */
            {
                printf("syntax error: close \" expected\n");
                free(value);
                free_token_list(&head);
                return NULL;
            }
            break;
        case STATE_SINGLE_QUOTES:
            value = (char*) realloc(value, ++i * sizeof(char));
            if (value == NULL)
            {
                perror("realloc");
                exit(ALLOC_ERR);
            }
            if (ch != '\'')
            {
                state = STATE_SINGLE_QUOTES;
                is_read = 1;
                value[i - 1] = (char) ch;
            }
            if (ch == '\'')
            {
                state = STATE_LOOP;
                is_read = 1;
                value[i - 1] = '\0';
                head = push_tail_token(head, LES_SINGLE_QUOTES, value);
                value = NULL;
                i = 0;
            }
            if (ch == EOF || ch == '\n') /* mb another state for error ? */
            {
                printf("syntax error: close \' expected\n");
                free(value);
                free_token_list(&head);
                return NULL;
            }
            break;
        case STATE_END:
            return head;
        }
    }
}

token_t *
parse_step_2(token_t *token_list_head)
{
    token_t *iter = NULL;
    token_t *tmp = NULL;

    for (iter = token_list_head; iter; iter = iter->next)
    {
        if (iter->lex == LEX_ID)
        {
            if (string_search_2_symbols(iter->value, '?', '*'))
                iter->lex = LEX_REGEX_TEMPLATE;
            else if (iter->value[0] == '2' && iter->value[1] == '\0' && \
                     iter->next != NULL && iter->next->lex == LEX_MORE)
            {
                if (iter == token_list_head)
                {
                    printf("before 2> must be id\n");
                    free_token_list(&token_list_head);
                    return NULL;
                }

                if (iter->next->next != NULL && iter->next->next->lex == LEX_MORE)
                {
                    tmp = iter->prev;
                    tmp->next = iter->next->next;
                    iter->next->next->prev = tmp;
                    free(iter->next);
                    free(iter->value);
                    free(iter);
                    iter = tmp;
                    iter->next->lex = LEX_TWO_MOREMORE;
                }
                else
                {
                    tmp = iter->prev;
                    tmp->next = iter->next;
                    iter->next->prev = tmp;
                    free(iter->value);
                    free(iter);
                    iter = tmp;
                    iter->next->lex = LEX_TWO_MORE;
                }
            }
        } /* end if lex id */
        else if (iter->lex == LEX_AND)
        {
            if (iter->next != NULL && iter->next->lex == LEX_AND)
            {
                if (iter == token_list_head)
                {
                    printf("before & must be id\n");
                    free_token_list(&token_list_head);
                    return NULL;
                }
                else
                {
                    tmp = iter->prev;
                    tmp->next = iter->next;
                    iter->next->prev = tmp;
                    free(iter);
                    iter = tmp;
                    iter->next->lex = LEX_ANDAND;
                }
            }
        } /* end if lex and */
        else if (iter->lex == LEX_MORE)
        {
            if (iter == token_list_head)
            {
                printf("before > must be id\n");
                free_token_list(&token_list_head);
                return NULL;
            }
            if (iter->next != NULL && iter->next->lex == LEX_MORE)
            {
                tmp = iter->prev;
                tmp->next = iter->next;
                iter->next->prev = tmp;
                free(iter->value);
                free(iter);
                iter = tmp;
                iter->next->lex = LEX_MOREMORE;
            }
        } /* end if lex more */
        else if (iter->lex == LEX_LESS && iter == token_list_head)
        {
            printf("before < must be id\n");
            free_token_list(&token_list_head);
            return NULL;
        }/* end if lex less */
    } /* end for */

    return token_list_head;
}

void
free_token_list(token_t **head)
{
    token_t *tmp = NULL, *iter = NULL;

    for (iter = *head; iter; iter = tmp)
    {
        tmp = iter->next;
        if (iter->value != NULL) free(iter->value);
        free(iter);
    }

    *head = NULL;
}

/*void
print_token_list(token_t *head)
{
    for (; head; head = head->next)
        printf("type = %d value = %s\n", head->lex, head->value);
}*/
