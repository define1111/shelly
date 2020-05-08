#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/passes.h"
#include "../include/parser.h"
#include "../include/command.h"
#include "../include/error_list.h"
#include "../include/pass_builtin_commands.h"
#include "../include/pass_open_files.h"

pass_return_code_t 
run_passes()
{
    passes_t current_pass;
    token_t *token_list_head = NULL;
    token_t **tokens_conveyor = NULL;
    command_t **commands = NULL;
    unsigned int conveyor_length = 0;
    unsigned int i, j;
    int (*pipe_fd)[2]; /* array of pointers to pipe_fd[2] */

    for (current_pass = PASS_TOKENIZATION; 1; ++current_pass)
    {
        switch (current_pass)
        {
        case PASS_TOKENIZATION:
        /* DESCRIPTION: this pass take pointer to list of tokens from stdin*/
            token_list_head = parse_step_1();
            if (token_list_head == NULL) return PASS_RET_CONTINUE;
            token_list_head = parse_step_2(token_list_head);
            if (token_list_head == NULL) return PASS_RET_CONTINUE;
            /* print_token_list(token_list_head); */
            break;
        case PASS_SPLIT_CONVEYOR:
        /* DESCRIPTION: this pass take pointer to array of list of tokens
           separeted by | token and free this token */
            tokens_conveyor = conv_parse(token_list_head);
            if (tokens_conveyor == NULL) return PASS_RET_CONTINUE;
            break;
        case PASS_GET_COMMANDS_FROM_CONVEYOR:
        /* DESCRIPTION: this pass take array of commands. Command_t - special struct for
           easy access to args, files. For example:
           $ ls -al | grep .txt | sort > file.txt interpreted as:
           commands[0]: args = {"ls", "-al", NULL}, in = NULL, out = NULL
           commands[1]: args = {"grep", ".txt", NULL}, in = NULL, out = NULL
           commands[2]: args = {"sort", NULL}, in = NULL, out = "file.txt" */
            conveyor_length = conv_len(tokens_conveyor);
            commands = (command_t**) malloc((conveyor_length + 1) * sizeof(command_t*));
            if (commands == NULL)
            {
                perror("malloc");
                exit(ALLOC_ERR);
            }
            for (i = 0; i < conveyor_length; ++i)
            {
                commands[i] = get_command(tokens_conveyor, i);
                if (commands[i] == NULL) 
                {
                    free_commands(commands);
                    return PASS_RET_CONTINUE;
                }
            }
            commands[i] = NULL;
            break;
        case PASS_OPEN_FILES_FOR_CONVEYOR:
            /* DESCRIPTION: actually we need open only two files
               for conveyor: fd_in for first command and fd_out
               for last command and fd_err for all commands */
            if (run_open_files(commands, tokens_conveyor, conveyor_length) == PASS_RET_CONTINUE)
                return PASS_RET_CONTINUE;
            break;
        case PASS_EXECUTE_BUILTIN_COMMAND:
            /* DESCRIPTION: run builtin command */
            commands[0]->builtin_command_type = detect_buitin_command_type(commands[0]);
            if (commands[0]->builtin_command_type != BUILTIN_COMMAND_NONE && \
                run_builtin_commands(commands, tokens_conveyor, &current_pass) == PASS_RET_SUCCESS)
                return PASS_RET_SUCCESS;
            break;
        case PASS_INIT_CONVEYOR:
            if (conveyor_length <= 1) break;
            pipe_fd = malloc(2 * (conveyor_length - 1) * sizeof(int));
            if (pipe_fd == NULL)
            {
                perror("malloc");
                exit(ALLOC_ERR);
            }
            break; 
        case PASS_EXECUTE_EXTERNAL_COMMAND:
            if (conveyor_length > 1)
            {
                for (i = 0; i < conveyor_length; ++i)
                {
                    if (i != conveyor_length - 1)
                        if (pipe(pipe_fd[i]) == -1)
                        {
                            perror("pipe");
                            exit(PIPE_ERR);
                        }

                    commands[i]->pid = fork();
                    if (commands[i]->pid == -1)
                    {
                        perror("fork");
                        exit(FORK_ERR);
                    }
                
                    if (commands[i]->pid == 0)
                    {
                        if (i == 0) /* first command */
                        {
                            if (commands[i]->fd_input_file != -1) 
                            {
                                dup2(commands[i]->fd_input_file, STDIN_FILENO);
                                close(commands[i]->fd_input_file);
                            }

                            if (commands[i]->fd_error_output_file != -1)
                            {
                                dup2(commands[i]->fd_error_output_file, STDERR_FILENO);
                                close(commands[i]->fd_error_output_file);
                            }

                            dup2(pipe_fd[i][1], STDOUT_FILENO);

                            for (j = 0; j < i; ++j)
                            {
                                close(pipe_fd[j][0]);
                                close(pipe_fd[j][1]);
                            }
                        }
                        else if (i == conveyor_length - 1) /* last command */
                        {
                            if (commands[i]->fd_output_file != -1) 
                            {
                                dup2(commands[i]->fd_output_file, STDOUT_FILENO);
                                close(commands[i]->fd_output_file);
                            }

                            if (commands[i]->fd_error_output_file != -1)
                            {
                                dup2(commands[i]->fd_error_output_file, STDERR_FILENO);
                                close(commands[i]->fd_error_output_file);
                            }

                            dup2(pipe_fd[i - 1][0], STDIN_FILENO);

                            for (j = 0; j < i; ++j)
                            {
                                close(pipe_fd[j][0]);
                                close(pipe_fd[j][1]);
                            }
                        }
                        else /* middle command */
                        {
                            if (commands[i]->fd_error_output_file != -1)
                            {
                                dup2(commands[i]->fd_error_output_file, STDERR_FILENO);
                                close(commands[i]->fd_error_output_file);
                            }

                            dup2(pipe_fd[i - 1][0], STDIN_FILENO);
                            dup2(pipe_fd[i][1], STDOUT_FILENO);
                            
                            for (j = 0; j < i; ++j)
                            {
                                close(pipe_fd[j][0]);
                                close(pipe_fd[j][1]);
                            }
                        }
                        execvp(commands[i]->args[0], commands[i]->args);
                        perror("exec");
                        exit(EXEC_ERR);
                    }
                }

                for (i = 0; i < conveyor_length - 1; ++i)
                {
                    close(pipe_fd[i][0]);
                    close(pipe_fd[i][1]);
                }

                for (i = 0; i < conveyor_length; ++i)
                    waitpid(commands[i]->pid, NULL, 0);
                
            }
            else
            {
                commands[0]->pid = fork();
                if (commands[0]->pid == -1)
                {
                    perror("fork");
                    exit(FORK_ERR);
                }   

                if (commands[0]->pid == 0)
                {
                    if (commands[0]->fd_input_file != -1) 
                    {
                        dup2(commands[0]->fd_input_file, STDIN_FILENO);
                        close(commands[0]->fd_input_file);
                    }
                    if (commands[0]->fd_output_file != -1) 
                    {
                        dup2(commands[0]->fd_output_file, STDOUT_FILENO);
                        close(commands[0]->fd_output_file);
                    }
                    if (commands[0]->fd_error_output_file != -1)
                    {
                        dup2(commands[0]->fd_error_output_file, STDERR_FILENO);
                        close(commands[0]->fd_error_output_file);
                    }
                    execvp(commands[0]->args[0], commands[0]->args);
                    perror("exec");
                    exit(EXEC_ERR);
                }
        
                waitpid(commands[0]->pid, NULL, 0);
            }
            break;
        case PASS_CLOSE_FILES_FOR_CONVEYOR:
            for (i = 0; i < conveyor_length; ++i)
            {
                if (commands[i]->fd_input_file != -1)
                    close(commands[i]->fd_input_file);
                if (commands[i]->fd_output_file != -1)
                    close(commands[i]->fd_output_file);
                if (commands[i]->fd_error_output_file != -1)
                    close(commands[i]->fd_error_output_file);
            }
            break;
        case PASS_FREE_ALLOCS:
            if (conveyor_length > 1) 
                free(pipe_fd); 
            free_conv(tokens_conveyor);
            free_commands(commands);
            break;
        case PASS_END:
            return PASS_RET_CONTINUE;
        } /* end switch */
    } /*end for */
}
