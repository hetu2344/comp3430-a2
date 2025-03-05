#include <stdio.h>
#include <stdlib.h>
#include "nqp_io.h"

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

    printf( "%s:\\> ", volume_label );
    while ( fgets( line_buffer, MAX_LINE_SIZE, stdin ) != NULL )
    {
        handle_cmd(line_buffer);
        printf( "%s:\\> ", volume_label );
    }

    return EXIT_SUCCESS;
}

/**
 * Splits a string into substrings based on a given delimiter.
 * 
 * @param string The input string.
 * @param split_char The delimiter character.
 * @return Array of strings, NULL-terminated.
 */
 char **split(const char *string, const char split_char) {
    int length = strlen(string);

    int num_words = 0;

    // Count occurrences of split_char to determine the number of words
    for (int i = 0; i < length; i++) {
        if (string[i] == split_char) {
            num_words++;
        }
    }

    num_words++; // Account for the last word

    // Allocate space for array of words (+1 for NULL termination)
    char **split_ary = (char **)malloc((num_words + 1) * sizeof(char *));

    char *copy = strdup(string);

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


void handle_cmd(const char* cmd){
    if(cmd != NULL){
        
    }
}

void process_ls_cmd(const char* file){
    fd = nqp_open(file);

    if (fd == NQP_FILE_NOT_FOUND) {
        fprintf(stderr, "%s not found\n", argv[2]);
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
            fprintf(stderr, "%s is not a directory\n", argv[2]);
        }

        nqp_close(fd);
    }
}
