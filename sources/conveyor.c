#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/conveyor.h"
#include "../include/parser.h"
#include "../include/command.h"
#include "../include/pass_builtin_commands.h"
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

void
run_conveyor(conveyor_t *conveyor, token_t **tokens_conveyor)
{
    unsigned int i, j;

    for (i = 0; i < conveyor->length; ++i)
    {
        switch(conveyor->commands[i]->builtin_command_type)
        {
        case BUILTIN_COMMAND_CD:
        case BUILTIN_COMMAND_EXIT:
            run_builtin_commands(conveyor, i, tokens_conveyor);
            break;
        default:
            break;
        }

        if (i != conveyor->length - 1)
            if (pipe(conveyor->pipe_fd[i]) == -1)
            {
                perror("pipe");
                exit(PIPE_ERR);
            }

        conveyor->commands[i]->pid = fork();
        if (conveyor->commands[i]->pid == -1)
        {
            perror("fork");
            exit(FORK_ERR);
        }

        if (conveyor->commands[i]->pid == 0)
        {
            if (i == 0) /* first command */
            {
                if (conveyor->commands[i]->fd_input_file != -1) 
                {
                    dup2(conveyor->commands[i]->fd_input_file, STDIN_FILENO);
                    close(conveyor->commands[i]->fd_input_file);
                }

                if (conveyor->commands[i]->fd_error_output_file != -1)
                {
                    dup2(conveyor->commands[i]->fd_error_output_file, STDERR_FILENO);
                    close(conveyor->commands[i]->fd_error_output_file);
                }

                if (conveyor->length > 1) dup2(conveyor->pipe_fd[i][1], STDOUT_FILENO);

                for (j = 0; j < i; ++j)
                {
                    close(conveyor->pipe_fd[j][0]);
                    close(conveyor->pipe_fd[j][1]);
                }
            }
            else if (i == conveyor->length - 1) /* last command */
            {
                if (conveyor->commands[i]->fd_output_file != -1) 
                {
                    dup2(conveyor->commands[i]->fd_output_file, STDOUT_FILENO);
                    close(conveyor->commands[i]->fd_output_file);
                }

                if (conveyor->commands[i]->fd_error_output_file != -1)
                {
                    dup2(conveyor->commands[i]->fd_error_output_file, STDERR_FILENO);
                    close(conveyor->commands[i]->fd_error_output_file);
                }

                dup2(conveyor->pipe_fd[i - 1][0], STDIN_FILENO);

                for (j = 0; j < i; ++j)
                {
                    close(conveyor->pipe_fd[j][0]);
                    close(conveyor->pipe_fd[j][1]);
                }
            }
            else /* middle command */
            {
                if (conveyor->commands[i]->fd_error_output_file != -1)
                {
                    dup2(conveyor->commands[i]->fd_error_output_file, STDERR_FILENO);
                    close(conveyor->commands[i]->fd_error_output_file);
                }

                dup2(conveyor->pipe_fd[i - 1][0], STDIN_FILENO);
                dup2(conveyor->pipe_fd[i][1], STDOUT_FILENO);
                
                for (j = 0; j < i; ++j)
                {
                    close(conveyor->pipe_fd[j][0]);
                    close(conveyor->pipe_fd[j][1]);
                }
            }
            
            switch(conveyor->commands[i]->builtin_command_type)
            {
            case BUILTIN_COMMAND_NONE:
                execvp(conveyor->commands[i]->args[0], conveyor->commands[i]->args);
                perror("exec");
                exit(EXEC_ERR);
            case BUILTIN_COMMAND_CD:
            case BUILTIN_COMMAND_EXIT:
                break;
            default:
                run_builtin_commands(conveyor, i, tokens_conveyor);
                break;
            }
        }
    }

    for (i = 0; i < conveyor->length - 1; ++i)
    {
        close(conveyor->pipe_fd[i][0]);
        close(conveyor->pipe_fd[i][1]);
    }

    /*if (conveyor->length > 1)*/
        for (i = 0; i < conveyor->length; ++i)
            waitpid(conveyor->commands[i]->pid, NULL, 0);
}

void
free_conveyor(conveyor_t *conveyor)
{
    free_commands(conveyor->commands);
    if (conveyor->pipe_fd != NULL) free(conveyor->pipe_fd);
    free(conveyor);
}
