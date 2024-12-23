#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "regex_to_nfa.h"

// Stack operations
#define stack_push(s) *stackp++ = s   // I didn't even know you could do this
#define stack_pop()   *--stackp     

// Forward declarations
nfa base_fragment(char);
nfa concatenate(nfa, nfa);
nfa alternate(nfa, nfa);
nfa kleene_closure(nfa);

nfa generate_nfa(char* expression) {
    nfa stack[100];
    nfa* stackp = stack;
    char *p; // current character in string

    state** pointers = malloc(128 * sizeof(state*));
    int num_states = 0;

    for (p = expression; *p != '\0'; p++) {
        nfa s, f1, f2;
        switch(*p) {
            case '/': // concatenation character
                f1 = stack_pop(); f2 = stack_pop();
                s = concatenate(f2, f1); // opposite order of pop()
                stack_push(s);
                break;

            case '*': ;
                f1 = stack_pop();
                s = kleene_closure(f1);
                stack_push(s);
                pointers[num_states++] = s.head;
                pointers[num_states++] = s.tail;
                break;

            case '|': ;
                f1 = stack_pop(); f2 = stack_pop();
                s = alternate(f1, f2);
                stack_push(s);
                pointers[num_states++] = s.head;
                pointers[num_states++] = s.tail;
                break;

            default:
                s = base_fragment(*p);
                stack_push(s);
                pointers[num_states++] = s.head;
                pointers[num_states++] = s.tail;
        }
    }
    if (stackp != &stack[1]) {
        fprintf(stderr, "NFA generation failed; exiting.\n");
        exit(1);
    }
    stack[0].num_states = num_states;
    stack[0].pointers = pointers;
    return stack[0];
}

/*
 * Generates an NFA for a single character
 */
nfa base_fragment(char c) {
    state* head = malloc(sizeof(state));
    state* tail = malloc(sizeof(state));

    head->num_outgoing = 1;
    head->types[0] = c;
    head->outgoing[0] = tail;
    head->terminated = false;

    tail->accepting = true;
    tail->num_outgoing = 0;
    tail->terminated = false;

    return (nfa){head, tail};
}

/*
 * Concatenates two NFAs
 */
nfa concatenate(nfa f1, nfa f2) {
    f1.tail->accepting = false;
    f1.tail->num_outgoing = 1;
    f1.tail->types[0] = EPSILON;
    f1.tail->outgoing[0] = f2.head;

    return (nfa){f1.head, f2.tail};
}

/*
 * Alternates two NFAs (union) 
 */
nfa alternate(nfa f1, nfa f2) {
    state* s0 = malloc(sizeof(state));
    s0->accepting = false;
    s0->num_outgoing = 2;
    s0->types[0] = EPSILON; 
    s0->types[1] = EPSILON;
    s0->outgoing[0] = f1.head;
    s0->outgoing[1] = f2.head;
    s0->terminated = false;

    state* sa = malloc(sizeof(state));
    sa->accepting = true;
    sa->num_outgoing = 0;
    sa->terminated = false;

    f1.tail->accepting = false;
    f1.tail->num_outgoing = 1;
    f1.tail->types[0] = EPSILON;
    f1.tail->outgoing[0] = sa;

    f2.tail->accepting = false;
    f2.tail->num_outgoing = 1;
    f2.tail->types[0] = EPSILON;
    f2.tail->outgoing[0] = sa;

    return (nfa){s0, sa};
}

/*
 * Kleene closure of an NFA 
 */
nfa kleene_closure(nfa f1) {
    state* sa = malloc(sizeof(state));
    sa->accepting = true;
    sa->num_outgoing = 0;
    sa->terminated = false;

    
    state* s0 = malloc(sizeof(state));
    s0->accepting = false;
    s0->num_outgoing = 2;
    s0->types[0] = EPSILON; s0->types[1] = EPSILON;
    s0->outgoing[0] = f1.head;
    s0->outgoing[1] = sa;
    s0->terminated = false;

    f1.tail->accepting = false;
    f1.tail->num_outgoing = 2;
    f1.tail->types[0] = EPSILON;
    f1.tail->types[1] = EPSILON;
    f1.tail->outgoing[0] = sa;
    f1.tail->outgoing[1] = f1.head;

    return (nfa){s0, sa};
}

void free_nfa(nfa n) {
    for (int i = 0; i < n.num_states; i++) {
        free(n.pointers[i]);
    }
    free(n.pointers);
}

#define ERROR_STATE     NULL 
regex_result evaluate_nfa(char* expression, FILE* file) {

    nfa regex = generate_nfa(expression);

    // Initialize array for tracking lines with matches
    int buffer_size = 100;
    int buffer_index = 0;
    int* out_lines = malloc(buffer_size * sizeof(int));

    // Initialize active states array
    state* active_states[128];
    int active_index = 0;
    char c;
    int line = 0;
    bool accept_line = false;

    // Add regex head (n0) to active states
    active_states[active_index++] = regex.head;

    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            for (int i = 0; i < active_index; i++) {
                active_states[i]->terminated = false;
            }
            if (accept_line) {
                out_lines[buffer_index++] = line;
                if (buffer_index == buffer_size) {
                    buffer_size *= 2;
                    out_lines = realloc(out_lines, buffer_size * sizeof(int));
                }
            }
            line++;
            active_index = 0;
            active_states[active_index++] = regex.head; // old states will be ignored and overwritten
            accept_line = false;
            continue;
        }
        else if (accept_line) { // match already found on this line
            continue;
        }
        // Collect all epsilon-connected states and add them to active_states
        // Then transition to new state (or error state)
        // Since states only have up to 2 incoming edges, each state can
        // only be added to active_states twice (at most)
        for (int i = 0; i < active_index; i++) {
            state* s = active_states[i];
            if (s->terminated) {
                continue;
            }
            state* new_state = ERROR_STATE;
            for (int j = 0; j < s->num_outgoing; j++) {
                if (s->outgoing[j]->terminated) {
                    continue;
                }
                else if (s->types[j] == EPSILON) {
                    active_states[active_index++] = s->outgoing[j]; 
                }
                else if (s->types[j] == c) {
                    new_state = s->outgoing[j];
                }
            }
            if (s->accepting || new_state != ERROR_STATE && new_state->accepting) { // if a match is found on current line
                accept_line = true;
                break;
            }
            else if (new_state == ERROR_STATE) { // terminate state if it goes to error state
                active_states[i]->terminated = true;
            }
            else { // set active_state[i] to next state if not erorr
                active_states[i] = new_state;
            }
        }
        int num_errors = 0;
        for (int i = 0; i < active_index; i++) { // count terminated (error) states
            if (active_states[i]->terminated) {
                num_errors++;
            }
        }
        if (num_errors == active_index) { // if all states are errors, restart NFA evaluation
            for (int i = 0; i < active_index; i++) {
                active_states[i]->terminated = false;
            }
            active_index = 0;
            active_states[active_index++] = regex.head; // old states will be ignored and overwritten
        }
    }
    free_nfa(regex);
    return (regex_result){.num_match_lines = buffer_index, .match_lines = out_lines};
}

/*
Barry | Adam
Barry////Adam///|
*/