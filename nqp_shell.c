#define _GNU_SOURCE  
#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "nqp_io.h"

char* curr_dir = NULL;
char* parent_dir = NULL;

/**
 * Splits a string into substrings based on a given delimiter, from start to end  (end not included)
 * 
 * @param string The input string.
 * @param split_char The delimiter character.
 * @return Array of strings, NULL-terminated.
 */
 char **split(const char *string, const char split_char , const int start, const int end) {
    // int length = strlen(string);

    int num_words = 0;

    // Count occurrences of split_char to determine the number of words
    for (int i = start; i < end; i++) {
        if (string[i] == split_char) {
            num_words++;
        }
    }

    num_words++; // Account for the last word

    // Allocate space for array of words (+1 for NULL termination)
    char **split_ary = (char **)malloc((num_words + 1) * sizeof(char *));

    char *copy = strndup(string, end - start);
    copy[end] = '\0';

    char *word = strtok(copy, (char[]){split_char, '\0'});

    for (int i = 0; i < num_words; i++) {
        if (word == NULL) {
            split_ary[i] = NULL; // Avoid accessing NULL
            break;
        }

        split_ary[i] = (char *)malloc(strlen(word) + 1);

        strcpy(split_ary[i], word);
        word = strtok(NULL, (char[]){split_char, '\0'}); // Get next token
    }

    split_ary[num_words] = NULL; // Null-terminate the array
    free(copy);                  // Free the duplicated string

    return split_ary;
}

int get_index_of(const char* string, const char target){
    char* target_ptr = strchr(string, target);

    return target_ptr != NULL ? target_ptr - string : -1;
}

// This method returns a malloc string which the caller function should free it
char* get_cmd_name(const char* user_input){
    // printf("user_input = %s\n", user_input);
    char* cmd = NULL;
    if(user_input != NULL){
        char* space_ptr = strchr(user_input, ' ');

        int len = space_ptr != NULL ? len = space_ptr - user_input : strlen(user_input);
        // printf("strlen = %lu\n", strlen(user_input));
        // printf("len = %d\n", len);
        cmd = malloc(len + 1);
        strncpy(cmd, user_input, len);
        cmd[len] = '\0';
        // printf("user_input: %s\n", user_input);
        // printf("cmd       : %s\n", cmd);
    }

    return cmd;
}

void process_ls_cmd(void){
    nqp_dirent entry = {0};
    // nqp_error err;
    int fd;
    ssize_t dirents_read;

    fd = nqp_open(curr_dir);

    if (fd == NQP_FILE_NOT_FOUND) {
        // fprintf(stderr, "%s not found\n", argv[2]);
    } else {
        while ((dirents_read = nqp_getdents(fd, &entry, 1)) > 0) {
            printf("%d %s", (int)(entry.inode_number), entry.name);

            if (entry.type == DT_DIR) {
                putchar('/');
            }

            putchar('\n');

            free(entry.name);
        }

        if (dirents_read == -1) {
            // fprintf(stderr, "%s is not a directory\n", argv[2]);
        }

        nqp_close(fd);
    }
}

// This function return malloc string, which caller has to free it
char* get_dir_name_for_cd(const char* user_input){
    char* dir_name = NULL;
    if(user_input != NULL){
        int len = strlen(user_input);
        char* space_ptr = strchr(user_input, ' ');

        if(space_ptr != NULL){
            dir_name = malloc(sizeof(char) * (len - (space_ptr - user_input)));
            strcpy(dir_name, space_ptr + 1);
            dir_name[len - (space_ptr - user_input) - 1] = '\0';
        }
        // printf("user_input: %s\n", user_input);
        // printf("cmd       : %s\n", cmd);
    }

    return dir_name;
}

void process_cd_cmd(const char* user_input){
    char* dir_name = get_dir_name_for_cd(user_input);
    (void) dir_name;
    if (dir_name == NULL){
        printf("Error: provide directory name in cd\n");
    } else if (strcmp(dir_name, "..") == 0){
        free(curr_dir);
        curr_dir = parent_dir;
        if (strcmp(curr_dir, "/") == 0){
            parent_dir = malloc(sizeof(char) * 2);
            strcpy(parent_dir, "/");
        } else {
            // int len = 
            // needs to implement this but first need to make a nested dir volume to do it
        }
    } else {
        int fd;
        int new_curr_dir_len = strlen(curr_dir) + strlen(dir_name) + 2; // +2, 1 for '/' and another for '\0'
        char* new_curr_dir = malloc(sizeof(char) * new_curr_dir_len); 
        // printf("malloc size = %lu\n", (strlen(curr_dir) + strlen(dir_name) + 2));
        strncpy(new_curr_dir, curr_dir, strlen(curr_dir));
        // new_curr_dir[strlen(curr_dir)] = '/';
        strcpy(new_curr_dir + strlen(curr_dir), dir_name);
        // printf("strlen size = %lu\n", strlen(new_curr_dir));
        new_curr_dir[new_curr_dir_len - 2] = '/';
        new_curr_dir[new_curr_dir_len - 1] = '\0';

        // printf("new_curr_dir: %s\n", new_curr_dir);
        fd = nqp_open(new_curr_dir);
        if(fd == NQP_FILE_NOT_FOUND){
            printf("Error: \"%s\" Directory does not exists\n", dir_name);
        } else {
            // checking if the fd is directory or not
            nqp_dirent entry = {0};
            int check = nqp_getdents(fd, &entry, 1);
            (void) entry;
            // check should be -ve for directory
            if(check < 0){
                printf("Error: \"%s\" is not a directory\n", dir_name);
            } else {
                free(parent_dir);
                parent_dir = curr_dir;
                curr_dir = new_curr_dir;
            }
        }
    }
}

void process_pwd_cmd(void){
    printf("%s\n", curr_dir);
}

void process_simple_exec_cmd(const char* user_input, char** envp){
    // load the program from the img to memory
    // using memfd_create
    char** split_arr = split(user_input, ' ', 0, strlen(user_input));
    if(split_arr == NULL){
        printf("Error: \"%s\", Invalid command\n", user_input);
    } else {
        int mem_fd = memfd_create(split_arr[0], 0);
        if(mem_fd < 0){
            printf("Error: Cannot create memory for \"%s\"\n", split_arr[0]);
        } else {
            int program_fd = nqp_open(split_arr[0]);

            if(program_fd == NQP_FILE_NOT_FOUND){
                printf("Error: \"%s\", program not available in current directory\n", split_arr[0]);
            } else{
                char read_buffer[256] = {0};
                ssize_t bytes_read = 0;

                while((bytes_read = nqp_read(program_fd, &read_buffer, 256)) > 0){
                    write(mem_fd, &read_buffer, bytes_read);
                }

                pid_t child_id = fork();

                if(child_id == 0){
                    // in child process,
                    // fexecve the program
                    fexecve(mem_fd, split_arr, envp);
                    perror("fexecve failed");
                    // fprintf(stderr, "fexecve failed: %s\n", strerror(errno));
                    printf("Error: Cannot execute the \"%s\" program\n", split_arr[0]);
                } else {
                    // in parent,
                    // wait for child process to exit                    
                    waitpid(child_id, NULL, 0);
                }
            }
        }

    }
}

void process_red_exev_cmd(const char* user_input, char** envp, int red_index){
    char** split_arr = split(user_input, '<', 0, strlen(user_input));
    if(split_arr == NULL){
        printf("Error: Invalid command\n");
    } else {
        // Assuming:
        // split_arr[0] = will have cmd string with argument for exec
        // split_arr[1] = will have the file name for redirection, also assuming this is only one file name
        // split_arr[2] = NULL
        
        int pipefd[2];
        int pipe_id = pipe(pipes);

        pid_t child_id = fork();

        if(child_id == 0){
            // in child process
            // exec the command
            close(pipefd[1]); //close write end
            char** cmd_argv = split(split_arr[0], ' ', 0, stelen(split_arr[0]));
            int mem_fd = memfd_create(cmd_argv[0], 0);
            if(mem_fd < 0){
                printf("Error: Cannot create memory for \"%s\"\n", cmd_argv[0]);
            } else {
                int program_fd = nqp_open(cmd_argv[0]);
                if (program_fd < 0){
                    printf("Error: \"%s\", program not available in current directory\n", split_arr[0]);
                } else {
                    char read_buffer[256] = {0};
                    ssize_t bytes_read = 0;

                    while((bytes_read = nqp_read(program_fd, &read_buffer, 256)) > 0){
                        write(mem_fd, &read_buffer, bytes_read);
                    }

                    dup2(pipefd[0], STDIN_FILENO); // set read end of pipe as stdin
                    fexecve(mem_fd, cmd_argv, envp);
                    perror("fexecve failed");
                    // fprintf(stderr, "fexecve failed: %s\n", strerror(errno));
                    printf("Error: Cannot execute the \"%s\" program\n", split_arr[0]);
                }
            }
        } else {
            // in parent process
        }
    }
}

void process_exec_cmd(const char* user_input, char** envp){
    char* redirec_ptr = strchr(user_input, '<');

    if (redirec_ptr != NULL){
        
    } else {
        process_simple_exec_cmd(user_input, envp);
    }
}

void handle_cmd(char* user_input, char** envp){
    if(user_input != NULL){
        // process_builtin_cmds(user_input);
        // process_ls_cmd(); 
        user_input[strlen(user_input) - 1] = '\0'; // get rid of \n in linux
        char *first_cmd = get_cmd_name(user_input);
        // printf("first_cmd = %s\n", first_cmd);

        if(strcmp(first_cmd, "ls") == 0){
            process_ls_cmd();
            free(first_cmd);
        } else if (strcmp(first_cmd, "cd") == 0){
            process_cd_cmd(user_input);
            free(first_cmd);
        } else if (strcmp(first_cmd, "pwd") == 0){
            process_pwd_cmd();
            free(first_cmd);
        } else {
            // printf("rest cmds are in implementation\n");
            process_exec_cmd(user_input, envp);
        }
    }
}

int main( int argc, char *argv[], char *envp[] )
{
    #define MAX_LINE_SIZE 256
    char line_buffer[MAX_LINE_SIZE] = {0};
    char *volume_label = NULL;
    nqp_error mount_error;

    (void) envp;

    if ( argc != 2 )
    {
        fprintf( stderr, "Usage: ./nqp_shell volume.img\n" );
        exit( EXIT_FAILURE );
    }

    mount_error = nqp_mount( argv[1], NQP_FS_EXFAT );

    if ( mount_error != NQP_OK )
    {
        if ( mount_error == NQP_FSCK_FAIL )
        {
            fprintf( stderr, "%s is inconsistent, not mounting.\n", argv[1] );
        }

        exit( EXIT_FAILURE );
    }

    volume_label = nqp_vol_label( );
    // printf("volume lable = %s\n", volume_label);

    curr_dir = malloc(sizeof(char) * 2);
    strcpy(curr_dir, "/");

    parent_dir = malloc(sizeof(char) * 2);
    strcpy(parent_dir, "/");

    // char input[] = "echo \"Hi!\"\n";
    // handle_cmd(input, envp);
    // return EXIT_SUCCESS;

    printf( "%s:\\> ", volume_label );
    while ( fgets( line_buffer, MAX_LINE_SIZE, stdin ) != NULL )
    {
        handle_cmd(line_buffer, envp);
        printf( "%s:\\> ", volume_label );
    }

    free(volume_label);
    return EXIT_SUCCESS;
}
