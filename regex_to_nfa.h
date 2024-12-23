#include <stdlib.h>
#include <stdbool.h>
#include "nfa.h"

#ifndef REGEX_TO_NFA_H
#define REGEX_TO_NFA_H

/*
 * Generates an NFA from a regex expression in postfix form
 * and using '/' as a concatenation delimiter 
 * 
 * @param   Regex expression
 * @return  Pointer to initial state
 */
nfa generate_nfa(char* expression);

/*
 * Frees all malloc'd states in an NFA 
 *
 * @param node  Head of NFA to be freed
 */
void free_nfa(nfa n);

typedef struct regex_result {
    int num_match_lines;
    int* match_lines;
} regex_result;

/*
 * Evaluates the regex NFA like grep and returns the line numbers containing matches
 * 
 * @param expression    Postfix regex expression
 * @param file          File pointer to be grepped
 * @return              Line numbers containing matches 
 */
regex_result evaluate_nfa(char* expression, FILE* file);

#endif