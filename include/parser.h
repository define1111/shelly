#ifndef SENTRY_PARSER_H
#define SENTRY_PARSER_H

enum lex_t { LEX_NONE = 0, LEX_ID, LEX_QUOTES, LEX_MORE, LEX_LESS, LEX_AND, LEX_CONV };
typedef enum lex_t lex_t;

struct token_t
{
    lex_t lex;
    char *value;
    struct token_t *next, *prev;
};

typedef struct token_t token_t;

enum state_lex_t { STATE_LOOP = 0, STATE_IN_ID, STATE_QUOTES, STATE_END};
typedef enum state_lex_t state_lex_t;

token_t *parse(void);
void free_token_list(token_t **head);
//void print_token_list(token_t *head);

#endif
