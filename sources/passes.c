#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/passes.h"
#include "../include/parser.h"
#include "../include/analyzer.h"
#include "../include/error_list.h"
#include "../include/builtin_commands.h"

pass_return_code_t 
run_passes()
{
    token_t *token_list_head = NULL;
    token_t **conveyor = NULL;
    command_t **commands = NULL;
    unsigned int conveyor_length = 0;
    unsigned int i;
    int fd_in = -1;
    int fd_out = -1;
    int fd_err = -1;
    int (*pipe_fd)[2]; /* array of pointers to pipe_fd[2] */
    pid_t process = -1;
    pid_t *pid_arr = NULL; /* he he */

    for (passes_t current_pass = PASS_START; 1; ++current_pass)
    {
        switch (current_pass)
        {
        case PASS_START:
        /* DESCRIPTION: useless pass, lol */
            break;
        case PASS_TOKENIZATION:
        /* DESCRIPTION: this pass take pointer to list of tokens from stdin*/
            token_list_head = parse_step_1();
            if (token_list_head == NULL) return PASS_RET_CONTINUE;
            token_list_head = parse_step_2(token_list_head);
            if (token_list_head == NULL) return PASS_RET_CONTINUE;
            //print_token_list(token_list_head);
            break;
        case PASS_SPLIT_CONVEYOR:
        /* DESCRIPTION: this pass take pointer to array of list of tokens
           separeted by | token and free this token */
            conveyor = conv_parse(token_list_head);
            if (conveyor == NULL) return PASS_RET_CONTINUE;
            break;
        case PASS_GET_COMMANDS_FROM_CONVEYOR:
        /* DESCRIPTION: this pass take array of commands. Command_t - special struct for
           easy access to args, input file and output file. For example: 
           $ ls -al | grep .txt | sort > file.txt interpreted as:
           commands[0]: args = {"ls", "-al", NULL}, in = NULL, out = NULL
           commands[1]: args = {"grep", ".txt", NULL}, in = NULL, out = NULL
           commands[2]: args = {"sort", NULL}, in = NULL, out = "file.txt" */
            conveyor_length = conv_len(conveyor);
            commands = (command_t**) malloc((conveyor_length + 1) * sizeof(command_t*));
            if (commands == NULL)
            {
                perror("malloc");
                exit(ALLOC_ERR);
            }
            for (i = 0; i < conveyor_length; ++i)
            {
                commands[i] = get_command(conveyor, i);
                if (commands[i] == NULL) 
                {
                    free_commands(commands);
                    return PASS_RET_CONTINUE;
                }
            }
            commands[i] = NULL;
            break;
        case PASS_EXECUTE_BUILTIN_COMMAND:
            /* DESCRIPTION: run builtin command */
            if (run_builtin_commands(commands, conveyor, &current_pass) == PASS_RET_SUCCESS)
                return PASS_RET_SUCCESS;
            break;
        case PASS_OPEN_FILES_FOR_CONVEYOR:
            /* DESCRIPTION: actually we need open only two files
               for conveyor: fd_in for first command and fd_out
               for last command and fd_err for all commands */
            if (commands[0]->in != NULL)
            {
                fd_in = open(commands[0]->in, O_RDONLY, S_IRUSR|S_IWUSR);
                if (fd_in == -1)
                {
                    printf("open error: can't open file %s\n", commands[0]->in);
                    free_conv(conveyor);
                    free_commands(commands);
                    return PASS_RET_CONTINUE;
                }
            }
            for (unsigned int j = 0; j < conveyor_length; ++j)
            {
                if (commands[j]->err_out != NULL)
                {
                    fd_err = open(commands[j]->err_out, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
                    if (fd_err == -1)
                    {
                        printf("open error: can't open file %s\n", commands[j]->err_out);
                        free_conv(conveyor);
                        free_commands(commands);
                        return PASS_RET_CONTINUE;
                    }
                }
            }
            if (commands[conveyor_length - 1]->output_type == OUTPUT_TYPE_REWRITE)
            {
                fd_out = open(commands[conveyor_length - 1]->out, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
                if (fd_out == -1)
                {
                    printf("open error: can't open file %s\n", commands[conveyor_length - 1]->out);
                    free_conv(conveyor);
                    free_commands(commands);
                    return PASS_RET_CONTINUE;
                }
            }
            else if (commands[conveyor_length - 1]->output_type == OUTPUT_TYPE_APPEND)
            {
                fd_out = open(commands[conveyor_length - 1]->out, O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);
                if (fd_out == -1)
                {
                    printf("open error: can't open file %s\n", commands[conveyor_length - 1]->out);
                    free_conv(conveyor);
                    free_commands(commands);
                    return PASS_RET_CONTINUE;
                }
            }
            break;
        case PASS_INIT_CONVEYOR:
            if (conveyor_length <= 1) break;
            pipe_fd = malloc(2 * (conveyor_length - 1) * sizeof(int));
            if (pipe_fd == NULL)
            {
                perror("malloc");
                exit(ALLOC_ERR);
            }
            pid_arr = malloc(conveyor_length * sizeof(pid_t));
            if (pid_arr == NULL)
            {
                perror("malloc");
                exit(ALLOC_ERR);
            }
            break; 
        case PASS_EXECUTE_EXTERNAL_COMMAND:
            if (conveyor_length > 1)
            {
                for (unsigned int i = 0; i < conveyor_length; ++i)
                {
                    if (i != conveyor_length - 1)
                        if (pipe(pipe_fd[i]) == -1)
                        {
                            perror("pipe");
                            exit(PIPE_ERR);
                        }

                    pid_arr[i] = fork();
                    if (pid_arr[i] == -1)
                    {
                        perror("fork");
                        exit(FORK_ERR);
                    }
                
                    if (pid_arr[i] == 0)
                    {
                        if (i == 0) /* first command */
                        {
                            if (fd_in != -1) 
                            {
                                dup2(fd_in, STDIN_FILENO);
                                close(fd_in);
                            }

                            if (fd_err != -1)
                            {
                                dup2(fd_err, STDERR_FILENO);
                                close(fd_err);
                            }

                            dup2(pipe_fd[i][1], STDOUT_FILENO);

                            for (unsigned int j = 0; j < i; ++j)
                            {
                                close(pipe_fd[j][0]);
                                close(pipe_fd[j][1]);
                            }
                        }
                        else if (i == conveyor_length - 1) /* last command */
                        {
                            if (fd_out != -1) 
                            {
                                dup2(fd_out, STDOUT_FILENO);
                                close(fd_out);
                            }

                            if (fd_err != -1)
                            {
                                dup2(fd_err, STDERR_FILENO);
                                close(fd_err);
                            }

                            dup2(pipe_fd[i - 1][0], STDIN_FILENO);

                            for (unsigned int j = 0; j < i; ++j)
                            {
                                close(pipe_fd[j][0]);
                                close(pipe_fd[j][1]);
                            }
                        }
                        else /* middle command */
                        {
                            if (fd_err != -1)
                            {
                                dup2(fd_err, STDERR_FILENO);
                                close(fd_err);
                            }

                            dup2(pipe_fd[i - 1][0], STDIN_FILENO);
                            dup2(pipe_fd[i][1], STDOUT_FILENO);
                            
                            for (unsigned int j = 0; j < i; ++j)
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

                for (unsigned int i = 0; i < conveyor_length - 1; ++i)
                {
                    close(pipe_fd[i][0]);
                    close(pipe_fd[i][1]);
                }

                for (unsigned int i = 0; i < conveyor_length; ++i)
                    waitpid(pid_arr[i], NULL, 0);
                
            }
            else
            {
                process = fork();
                if (process == -1)
                {
                    perror("fork");
                    exit(FORK_ERR);
                }   

                if (process == 0)
                {
                    if (fd_in != -1) 
                    {
                        dup2(fd_in, STDIN_FILENO);
                        close(fd_in);
                    }
                    if (fd_out != -1) 
                    {
                        dup2(fd_out, STDOUT_FILENO);
                        close(fd_out);
                    }
                    if (fd_err != -1)
                    {
                        dup2(fd_err, STDERR_FILENO);
                        close(fd_err);
                    }
                    execvp(commands[0]->args[0], commands[0]->args);
                    perror("exec");
                    exit(EXEC_ERR);
                }
        
                waitpid(process, NULL, 0);
            }
            break;
        case PASS_FREE_ALLOCS:
            if (conveyor_length > 1) 
            {
                free(pipe_fd);
                free(pid_arr);
            }
            free_conv(conveyor);
            free_commands(commands);
            break;
        case PASS_END:
            return PASS_RET_CONTINUE;
        } /* end switch */
    } /*end for */
}