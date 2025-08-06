#include <stdio.h>
#include <stdlib.h>

typedef struct equilibrium_ {
  int label;
  double prob;
  struct equilibrium_* next;
} equilibrium;

typedef struct eqlist_ {
  equilibrium* eq;
  struct eqlist_* next;
} eqlist;

//Adds a strategy to an existent equilibrium, or creates a new one
equilibrium* add_strategy(equilibrium*,int,double);

//Print an equilibrium on FILE
void print_equilibrium(equilibrium*,FILE*);
void print_equilibrium_gambit(equilibrium*,int,int,FILE*);

//Lexicographically compares two equilibria
int lex_comp(equilibrium*,equilibrium*);

//Checks if the equilibrium is the artificial equilibrium
int is_artificial(equilibrium*);

//Return the support size of the equilibrium
int eq_size(equilibrium*);

//Frees the memory occupied by an equilibrium
void free_equilibrium(equilibrium*);

//Searchs for an equilibrium in eqlist. If it not finds it,
//it adds it, and puts 0 in found
eqlist* search_add_equilibrium(eqlist*,equilibrium*,int *found);

//Prints an equilibrium list on FILE
void print_eqlist(eqlist*,FILE*);
void print_eqlist_gambit(eqlist*,int,int,FILE*);

//Frees the memory occupied by a list of equilibria
void free_eqlist(eqlist*);
