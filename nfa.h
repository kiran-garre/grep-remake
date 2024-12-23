#ifndef DFA_H
#define DFA_H

#define ALPHABET_SIZE 128
#define EPSILON       128

#include <stdbool.h>

typedef struct state {
    bool accepting;
    int num_outgoing;
    unsigned char types[ALPHABET_SIZE];
    struct state* outgoing[ALPHABET_SIZE];

    bool terminated;
} state;

typedef struct nfa {
    state* head;
    state* tail;
    state** pointers;
    int num_states;
} nfa;

typedef struct dfa {
    int num_states;
    unsigned char** transitions;
    int accepting_states[ALPHABET_SIZE];
} dfa;

#endif