#ifndef _SENTRY_PARSER_H
#define _SENTRY_PARSER_H

enum lex_t 
{ 
    LEX_NONE = 0, LEX_ID, /* identifier */
    LES_SINGLE_QUOTES, /* ' */ LEX_DOUBLE_QUOTES, /* " */
    LEX_MORE, /* > */ LEX_MOREMORE, /* >> */ LEX_TWO_MORE, /* 2> */ LEX_TWO_MOREMORE, /* 2>> */
    LEX_LESS, /* < */  
    LEX_AND, /* & */
    LEX_BACKSLAH, /* \ */ LEX_SHARP, /* # */
    LEX_ANDAND, /* && */ LEX_CONV, /* | */ LEX_SEMICOLON, /* ; */
    LEX_REGEX_TEMPLATE /* * ? */
};
typedef enum lex_t lex_t;

struct token_t
{
    lex_t lex;
    char *value;
    struct token_t *next, *prev;
};
typedef struct token_t token_t;

enum state_lex_t { STATE_LOOP = 0, STATE_IN_ID, STATE_SINGLE_QUOTES, STATE_DOUBLE_QUOTES, STATE_END};
typedef enum state_lex_t state_lex_t;

token_t *parse_step_1(void);
token_t *parse_step_2(token_t *token_list_head);
void free_token_list(token_t **head);
/*void print_token_list(token_t *head);*/

#endif
