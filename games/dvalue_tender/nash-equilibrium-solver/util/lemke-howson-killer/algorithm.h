#pragma once
#include "bimatrix.h"

double* lemke_howson_gen(double*** tableaus, double** bimatrix, int dim1, int dim2, int pivot);

// eqlist* all_lemke_gen(double*** tableaus, double** bimatrix, int dim1, int dim2, int taboo,
//                       eqlist*);
