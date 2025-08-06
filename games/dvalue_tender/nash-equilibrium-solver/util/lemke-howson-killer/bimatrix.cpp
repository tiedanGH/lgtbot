#include "bimatrix.h"

/*
  Creates (and allocates necessary memory) the two tableaus needed by the algorithm,
  starting from the bimatrix. 
*/

double*** create_systems(double** bimatrix, int dim1, int dim2) {  
  int i, j;
  
  double*** tableaus = (double***) malloc( 2 * sizeof(double**) );

  //Memory allocation for the two tableaus
  
  tableaus[0] = (double**) malloc( dim1 * sizeof(double*) );
  for(i = 0; i < dim1; i++) {
    tableaus[0][i] = (double*) calloc( (2 + dim1 + dim2), sizeof(double) );
  }
  tableaus[1] = (double**) malloc( dim2 * sizeof(double*) );
  for(i = 0; i < dim2; i++) {
    tableaus[1][i] = (double*) calloc( (2 + dim1 + dim2), sizeof(double) );
  }
  
  /*
    Initialization of the two tableaus. The first column represents the index of the variable,
    with the convention that a negative number represents the slack variable associated with
    the corresponding positive index variable. The second column is the actual first column of
    the tableau, and represent, during the execution of the algorithm, the value of the variable
    in basis for that row.
  */
  
  for (i = 0; i < dim1; i++) {
    tableaus[0][i][0] = - i - 1.0;
    tableaus[0][i][1] = 1.0;
  }
  for (i = 0; i < dim2; i++) {
    tableaus[1][i][0] = - i - dim1 - 1.0;
    tableaus[1][i][1] = 1.0;
  }

  /*
    We now only need to copy the bimatrix in the correct cells in the tableau.
  */
  for (i = 0; i < dim1; i++ ) {
    for (j = (2 + dim1); j<(dim1+dim2+2); j++) {
      tableaus[0][i][j] = - bimatrix[i][j - 2 - dim1];
    }
  }
  for (i = 0; i < dim2; i++) {
    for (j =  (2 + dim2); j<(dim1+dim2+2); j++) {
      tableaus[1][i][j] = - bimatrix[dim1 + ( j - 2 - dim2)][i];
    }
  }

  return tableaus;
}

void view_bimatrix_gen(double** bimatrix, int dim1, int dim2, FILE *f) {
  int i, j;

  fprintf(f,"Bimatrix following:\n\nPlayer A:\n");
  for(i = 0; i < dim1; i++) {
    fprintf(f,"\n");
    for( j = 0; j < dim2; j++) {
      fprintf(f,"%lf ",bimatrix[i][j]);
    }
  }

  fprintf(f,"\n\nPlayer B:\n");
  for(i = dim1; i < 2 * dim1; i++) {
    fprintf(f,"\n");
    for( j = 0; j < dim2; j++) {
      fprintf(f,"%lf ",bimatrix[i][j]);
    }
  }

  fprintf(f,"\n\n");
}

void view_tableau_gen(double** tableau, int dim1, int dim2, FILE *f) {
  int i, j;

  for( i = 0; i < dim1; i++ ) {
    fprintf(f,"\n");
    for( j = 0; j < (2 + dim1 + dim2); j++) {
      fprintf(f,"%lf ",tableau[i][j]);
    }
  }

  fprintf(f,"\n");
}

/*
  Due to some facts we assume in our implementation of the algorithm, we need all of the payoffs to be positive, so
  we simply add an offset to all payoffs, thus having them all > 0.
*/

void positivize_bimatrix(double** bimatrix, int dim1, int dim2, double minimo) {
  int i, j;

  for(i = 0; i < (2 * dim1); i++) {
    for(j = 0; j < dim2; j++) {
      bimatrix[i][j] -= (minimo - 1.0);
    }
  }
}

/*
  Tells if strategy 'strategy' is in base (looking at the current tableau). If not, it returns the same strategy, if it's in base,
  it returns the corresponding slack variable. This is needed by all_lemke, because after each execution of the LH algorithm
  we pivot on every variable from 1 to dim1+dim2, without knowing if that variable is in fact in base or not.
*/

int get_pivot_gen(double*** tableaus, int dim1, int dim2, int strategy) {
  int i;

  for(i = 0; i < dim1; i++) {
    if( tableaus[0][i][0] == strategy )
      return -strategy;
  }

  for(i = 0; i < dim2; i++) {
    if( tableaus[1][i][0] == strategy)
      return -strategy;
  }

  return strategy;
}

//Returns the tableau that contains the given strategy

int get_tableau(int dim1, int dim2, int strategy) {
  if ( strategy > dim1 || (strategy < 0 && strategy >= -dim1) )
    return 0;
  if ( strategy < -dim1 || (strategy > 0 && strategy <= dim1) ) 
    return 1;
  
  return -1;
}

//Returns the column that corresponds to the given strategy

int get_column(int dim1, int dim2, int strategy) {
  
  if( strategy > 0 && strategy <= dim1 ) {
    return (1 + dim2 + strategy );
  }
  if( strategy > 0 && strategy > dim1 ) {
    return (1 + dim1 + strategy - dim1);
  }
  if( strategy < 0 && strategy >= -dim1) {
    return (1 - strategy );
  }
  
  return ( 1 - strategy - dim1 );
}

void free_tableaus(double*** tableaus, int dim1, int dim2) {
  int i;

  for(i=0; i < dim1; i++) {
    free(tableaus[0][i]);
  }
  free(tableaus[0]);

  for(i=0; i < dim2; i++) {
    free(tableaus[1][i]);
  }
  free(tableaus[1]);

  free(tableaus);
}

void free_bimatrix(double** bimatrix, int dim1, int dim2) {
  int i;

  for(i=0; i < (2*dim1); i++) {
    free(bimatrix[i]);
  }
  free(bimatrix);
}
