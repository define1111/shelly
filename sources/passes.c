#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/passes.h"
#include "../include/parser.h"
#include "../include/command.h"
#include "../include/conveyor.h"
#include "../include/error_list.h"
#include "../include/pass_builtin_commands.h"
#include "../include/pass_open_files.h"

pass_return_code_t 
run_passes()
{
    passes_t current_pass;
    token_t *token_list_head = NULL; /* list of all tokens */
    token_t **tokens_conveyor = NULL; /* array of lists of tokens separeted by | */
    conveyor_t *conveyors = NULL; /* array of conveyors */
    unsigned int i, j;

    for (current_pass = PASS_TOKENIZATION; 1; ++current_pass)
    {
        switch (current_pass)
        {
        case PASS_TOKENIZATION:
        /* DESCRIPTION: this pass take list of all tokens from stdin*/
            token_list_head = parse_step_1();
            if (token_list_head == NULL) return PASS_RET_CONTINUE;
            token_list_head = parse_step_2(token_list_head);
            if (token_list_head == NULL) return PASS_RET_CONTINUE;
            /* print_token_list(token_list_head); */
            break;
        case PASS_SPLIT_TOKENS_CONVEYOR:
        /* DESCRIPTION: this pass take pointer to array of list of tokens
           separeted by | token and free this token */
            tokens_conveyor = tokens_conveyor_parse(token_list_head);
            if (tokens_conveyor == NULL) return PASS_RET_CONTINUE;
            break;
        case PASS_INIT_CONVEYOR:
            conveyors = (conveyor_t*) malloc(sizeof(conveyor_t));
            if (conveyors == NULL)
            {
                perror("malloc");
                exit(ALLOC_ERR);
            }
            conveyors->length = get_conveyor_length_from_tokens(tokens_conveyor);
            if (conveyors->length <= 1) break;
            conveyors->pipe_fd = malloc(2 * (conveyors->length - 1) * sizeof(int));
            if (conveyors->pipe_fd == NULL)
            {
                perror("malloc");
                exit(ALLOC_ERR);
            }
            break; 
        case PASS_GET_COMMANDS_FROM_TOKENS_CONVEYOR:
        /* DESCRIPTION: this pass take array of commands. Command_t - special struct for
           easy access to args, files. For example:
           $ ls -al | grep .txt | sort > file.txt interpreted as:
           commands[0]: args = {"ls", "-al", NULL}, in = NULL, out = NULL
           commands[1]: args = {"grep", ".txt", NULL}, in = NULL, out = NULL
           commands[2]: args = {"sort", NULL}, in = NULL, out = "file.txt" */
            conveyors->commands = (command_t**) malloc((conveyors->length + 1) * sizeof(command_t*));
            if (conveyors->commands == NULL)
            {
                perror("malloc");
                exit(ALLOC_ERR);
            }
            for (i = 0; i < conveyors->length; ++i)
            {
                conveyors->commands[i] = get_command(tokens_conveyor, i);
                if (conveyors->commands[i] == NULL) 
                {
                    free_commands(conveyors->commands);
                    return PASS_RET_CONTINUE;
                }
            }
            conveyors->commands[i] = NULL;
            break;
        case PASS_OPEN_FILES_FOR_CONVEYOR:
            /* DESCRIPTION: actually we need open only two files
               for conveyor: fd_in for first command and fd_out
               for last command and fd_err for all commands */
            if (run_open_files(conveyors, tokens_conveyor) == PASS_RET_CONTINUE)
                return PASS_RET_CONTINUE;
            break;
        case PASS_DETECT_BUILTIN_COMMANDS:
            /* DESCRIPTION: before we can run conveyor we have to
               detect type of all commands: is it builtin or not? */
            for (i = 0; i < conveyors->length; ++i)
                conveyors->commands[i]->builtin_command_type = detect_buitin_command_type(conveyors->commands[i]);  
            break;   
        case PASS_EXECUTE_CONVEYOR:
            /* empty */
            break;
        case PASS_EXECUTE_BUILTIN_COMMAND:
            /* DESCRIPTION: run builtin command */
            if (conveyors->commands[0]->builtin_command_type != BUILTIN_COMMAND_NONE && \
                run_builtin_commands(conveyors->commands, tokens_conveyor, &current_pass) == PASS_RET_SUCCESS)
                return PASS_RET_SUCCESS;
            break;
        case PASS_EXECUTE_EXTERNAL_COMMAND:
            if (conveyors->length > 1)
            {
                for (i = 0; i < conveyors->length; ++i)
                {
                    if (i != conveyors->length - 1)
                        if (pipe(conveyors->pipe_fd[i]) == -1)
                        {
                            perror("pipe");
                            exit(PIPE_ERR);
                        }

                    conveyors->commands[i]->pid = fork();
                    if (conveyors->commands[i]->pid == -1)
                    {
                        perror("fork");
                        exit(FORK_ERR);
                    }
                
                    if (conveyors->commands[i]->pid == 0)
                    {
                        if (i == 0) /* first command */
                        {
                            if (conveyors->commands[i]->fd_input_file != -1) 
                            {
                                dup2(conveyors->commands[i]->fd_input_file, STDIN_FILENO);
                                close(conveyors->commands[i]->fd_input_file);
                            }

                            if (conveyors->commands[i]->fd_error_output_file != -1)
                            {
                                dup2(conveyors->commands[i]->fd_error_output_file, STDERR_FILENO);
                                close(conveyors->commands[i]->fd_error_output_file);
                            }

                            dup2(conveyors->pipe_fd[i][1], STDOUT_FILENO);

                            for (j = 0; j < i; ++j)
                            {
                                close(conveyors->pipe_fd[j][0]);
                                close(conveyors->pipe_fd[j][1]);
                            }
                        }
                        else if (i == conveyors->length - 1) /* last command */
                        {
                            if (conveyors->commands[i]->fd_output_file != -1) 
                            {
                                dup2(conveyors->commands[i]->fd_output_file, STDOUT_FILENO);
                                close(conveyors->commands[i]->fd_output_file);
                            }

                            if (conveyors->commands[i]->fd_error_output_file != -1)
                            {
                                dup2(conveyors->commands[i]->fd_error_output_file, STDERR_FILENO);
                                close(conveyors->commands[i]->fd_error_output_file);
                            }

                            dup2(conveyors->pipe_fd[i - 1][0], STDIN_FILENO);

                            for (j = 0; j < i; ++j)
                            {
                                close(conveyors->pipe_fd[j][0]);
                                close(conveyors->pipe_fd[j][1]);
                            }
                        }
                        else /* middle command */
                        {
                            if (conveyors->commands[i]->fd_error_output_file != -1)
                            {
                                dup2(conveyors->commands[i]->fd_error_output_file, STDERR_FILENO);
                                close(conveyors->commands[i]->fd_error_output_file);
                            }

                            dup2(conveyors->pipe_fd[i - 1][0], STDIN_FILENO);
                            dup2(conveyors->pipe_fd[i][1], STDOUT_FILENO);
                            
                            for (j = 0; j < i; ++j)
                            {
                                close(conveyors->pipe_fd[j][0]);
                                close(conveyors->pipe_fd[j][1]);
                            }
                        }
                        execvp(conveyors->commands[i]->args[0], conveyors->commands[i]->args);
                        perror("exec");
                        exit(EXEC_ERR);
                    }
                }

                for (i = 0; i < conveyors->length - 1; ++i)
                {
                    close(conveyors->pipe_fd[i][0]);
                    close(conveyors->pipe_fd[i][1]);
                }

                for (i = 0; i < conveyors->length; ++i)
                    waitpid(conveyors->commands[i]->pid, NULL, 0);
                
            }
            else
            {
                conveyors->commands[0]->pid = fork();
                if (conveyors->commands[0]->pid == -1)
                {
                    perror("fork");
                    exit(FORK_ERR);
                }   

                if (conveyors->commands[0]->pid == 0)
                {
                    if (conveyors->commands[0]->fd_input_file != -1) 
                    {
                        dup2(conveyors->commands[0]->fd_input_file, STDIN_FILENO);
                        close(conveyors->commands[0]->fd_input_file);
                    }
                    if (conveyors->commands[0]->fd_output_file != -1) 
                    {
                        dup2(conveyors->commands[0]->fd_output_file, STDOUT_FILENO);
                        close(conveyors->commands[0]->fd_output_file);
                    }
                    if (conveyors->commands[0]->fd_error_output_file != -1)
                    {
                        dup2(conveyors->commands[0]->fd_error_output_file, STDERR_FILENO);
                        close(conveyors->commands[0]->fd_error_output_file);
                    }
                    execvp(conveyors->commands[0]->args[0], conveyors->commands[0]->args);
                    perror("exec");
                    exit(EXEC_ERR);
                }
        
                waitpid(conveyors->commands[0]->pid, NULL, 0);
            }
            break;
        case PASS_CLOSE_FILES_FOR_CONVEYOR:
            for (i = 0; i < conveyors->length; ++i)
            {
                if (conveyors->commands[i]->fd_input_file != -1)
                    close(conveyors->commands[i]->fd_input_file);
                if (conveyors->commands[i]->fd_output_file != -1)
                    close(conveyors->commands[i]->fd_output_file);
                if (conveyors->commands[i]->fd_error_output_file != -1)
                    close(conveyors->commands[i]->fd_error_output_file);
            }
            break;
        case PASS_FREE_ALLOCS:
            if (conveyors->length > 1) 
                free(conveyors->pipe_fd); 
            free_tokens_conveyor(tokens_conveyor);
            free_commands(conveyors->commands);
            free(conveyors);
            break;
        case PASS_END:
            return PASS_RET_CONTINUE;
        } /* end switch */
    } /*end for */
}
