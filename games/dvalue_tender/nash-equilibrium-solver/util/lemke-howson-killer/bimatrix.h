#pragma once
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "equilibria.h"

typedef struct sh_tab {
  int row;
  int label;
  struct sh_tab* next;
} sh_tableau;

// Creates the tableaus starting from the bimatrix
double*** create_systems(double** bimatrix, int dim1, int dim2);

// Adds an offset to all payoffs to have them positive
void positivize_bimatrix(double** bimatrix, int dim1, int dim2, double min);

// Debug output
void view_bimatrix_gen(double**, int dim1, int dim2, FILE*);
void view_tableau_gen(double**, int dim1, int dim2, FILE*);

// Creates a copy of the system
double** system_copy(double**, int);

// Tells if strategy 'strategy' is in the current tableau's base.
int get_pivot_gen(double*** tableaus, int dim1, int dim2, int strategy);

// Returns the tableau in wich the strategy is contained
int get_tableau(int dim1, int dim2, int strategy);

// Returns the column that corresponds to the given strategy
int get_column(int dim1, int dim2, int strategy);

// Memory managment functions
void free_tableaus(double*** tableaus, int dim1, int dim2);
void free_bimatrix(double** bimatrix, int dim1, int dim2);
