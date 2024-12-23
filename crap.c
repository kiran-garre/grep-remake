#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "nfa.h"
#include "regex_to_nfa.h"

int main(int argc, char* argv[]) {
    
    if (argc < 3) {
        printf("Specify regex expression and filename.\n");
        exit(0);
    }

    char* expression = argv[1];
    char* filename = argv[2];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file: \"%s\"", filename);
        exit(1);
    }
    
    regex_result result = evaluate_nfa(expression, file);

    int max_line_length = 256;
    char* buffer = calloc(max_line_length, sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed; exiting");
        exit(1);
    }
    char c;
    int char_idx = 0;
    int line_counter = 0;
    int i = 0;
    
    rewind(file);
    while ((c = fgetc(file)) != EOF) {
        if (char_idx >= max_line_length - 1) {
            max_line_length *= 2;
            buffer = realloc(buffer, sizeof(char) * max_line_length);
        }
        if (c == '\n') {
            if (result.match_lines[i] == line_counter) {
                printf("%s\n", buffer);
                i++;
                if (i == result.num_match_lines) {
                    break;
                }   
            }
            line_counter++;
            memset(buffer, '\0', char_idx * sizeof(char));
            char_idx = 0;
        }
        else {
            buffer[char_idx++] = c;
        }
    }
    fclose(file);
    free(buffer);
    free(result.match_lines);
    return 0;
}