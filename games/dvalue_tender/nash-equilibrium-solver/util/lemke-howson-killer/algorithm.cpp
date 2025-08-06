#include "bimatrix.h"

#define eps 1e-8

double* lemke_howson_gen(double*** tableaus, double** bimatrix, int dim1, int dim2,
                         int startpivot) {
  int newpivot;
  double coeff, min, val;
  int i, j, index = 0;
  int updated;

  /*
    startpivot is the index of the variable we want to pivot on. get_pivot determines, looking at
    the tableau, if we want the real strategy to enter the basis, or the corresponding complementary
    variable. This happens, for example, when we don't start pivoting from the artificial
    equilibrium, but from an actual one, having in basis some real strategies: if we pivot on them,
    we want the complementary variable to enter the basis.
  */
  int pivot = get_pivot_gen(tableaus, dim1, dim2, startpivot);

  for (int iterations = 0; iterations < 3000; iterations++) {
    min = 0.0;
    updated = 0;

    // ntab is the tableau we are in (0 or 1)
    int ntab = get_tableau(dim1, dim2, pivot);

    // nlines is the number of rows of the tableau we are in (dim1 or dim2)
    int nlines = ntab == 0 ? dim1 : dim2;

    // column is the tableau column wich corresponds to the variable we are pivoting on
    int column = get_column(dim1, dim2, pivot);

    /*
      Minimum ratio test: we choose the index of the row in our tableau, for which the coefficient
      of the variable entering the basis is less than zero (if it's > 0 we cannot choose this row)
      and so that we minimize the ratio between the value of the variable in basis (
      tableaus[ntab][i][1] ) and the coefficient of the variable entering the basis
      (tableaus[ntab][i][column])
    */
    for (i = 0; i < nlines; i++) {
      if (tableaus[ntab][i][column] > -eps)  // We check that the coefficient is > 0
        continue;

      val = -(long double)tableaus[ntab][i][1] / tableaus[ntab][i][column];  // Ratio

      if (!updated ||
          val < (min - eps)) {  // We update the index of the row following the minimum ratio.
                                // Updated checks if it's the first feasible row we are trying
        min = val;
        updated = 1;
        index = i;
      }
    }
    /*
      If we didn't update the index of our row, this means there isn't a row for which the
      coefficient of the variable entering the basis if less than zero. This cannot happen, so if we
      are in this condition, we got something wrong.
    */
    // assert(updated != 0);
    if(!updated) break;

    // Finally we choose what variable will go out of the basis
    newpivot = (int)tableaus[ntab][index][0];

    /*
      Now we know what variable will go out of the basis, so we only need to do two things:
      - Solve the equation we chose with the minimum ratio test, updating the variable in basis
      - Solve all other equations of the tableau, updating all the coefficients.

      So the first step is to update the row chosen with the minimum ratio test: we update
      tableaus[ntab][index][0], which tells what variable is in basis and we calculate the
      coefficient we will divide all other coefficient with.
    */

    tableaus[ntab][index][get_column(dim1, dim2, newpivot)] = -1;
    tableaus[ntab][index][0] = pivot;
    coeff = -tableaus[ntab][index][column];

    /*
       Then we update the whole row, and we put the coefficient of variable entering basis to zero.
    */

    for (i = 1; i < (dim1 + dim2 + 2); i++) tableaus[ntab][index][i] /= coeff;
    tableaus[ntab][index][column] = 0;

    /*
      The second step is to solve all other equations in the tableau:
      - We check if the coefficient of the variable entering in basis in this row is nonzero
      - If so, we update the coefficients, and set to zero the coefficient of the variable entering
      basis
    */

    for (i = 0; i < nlines; i++) {
      if (tableaus[ntab][i][column] < -eps || tableaus[ntab][i][column] > eps) {
        for (j = 1; j < (dim1 + dim2 + 2); j++) {
          tableaus[ntab][i][j] += (long double)tableaus[ntab][i][column] * tableaus[ntab][index][j];
        }
        tableaus[ntab][i][column] = 0;
      }
    }

    /*
      Following the complementary pivoting rule, the new variable to pivot on is the complementary
      of the old variable
    */

    pivot = -newpivot;

    /*
      We stop the execution of the algorithm when either we pivot on the first variable we pivoted
      on (that strategy is leaving the basis), or on the complement of that variable (that
      complement is leaving the basis). In both cases, we are not in a k-almost complete
      equilibrium, but in an actual Nash equilibrium.
    */

    if (newpivot == startpivot || newpivot == -startpivot) break;
  }

  double tot1 = 0.0;
  double tot2 = 0.0;

  /*
    The only thing to do is to normalize the vector of strategy, thus obtaining sum of 1 for the
    probabilities.
  */
  for (i = 0; i < dim1; i++)
    if (tableaus[0][i][0] > 0) tot1 += tableaus[0][i][1];
  for (i = 0; i < dim2; i++)
    if (tableaus[1][i][0] > 0) tot2 += tableaus[1][i][1];

  // We create the actual equilibrium data structure with the normalized strategies
  double* eq = (double *)calloc(dim1 + dim2, sizeof(double));
  for (i = 0; i < dim1; i++) {
    if (tableaus[0][i][0] > 0) {
      eq[(int)round(tableaus[0][i][0]) - 1] = tableaus[0][i][1] / tot1;
    }
  }
  for (i = 0; i < dim2; i++) {
    if (tableaus[1][i][0] > 0) {
      eq[(int)round(tableaus[1][i][0]) - 1] = tableaus[1][i][1] / tot2;
    }
  }

  return eq;
}

/*
  This algorithm enumerates alla equilibria reachable by the Lemke-Howson Algorithm. Starting from
  one known equilibrium (the artificial one), the algorithm pivots on all strategies, stores the
  equilibrium found on a list, and if the equilibrium hadn't been found before, calls the algorithm
  recursively starting from that point. The idea for this implementation comes from the All_Lemke
  function contained in GAMBIT game-theory software, for the lcp tool.
*/

// eqlist* all_lemke_gen(double*** tableaus, double** bimatrix, int dim1, int dim2, int taboo,
//                       eqlist* lista) {
//   int pivot, found;

//   /*
//     Each execution of this algorithm has two parameters: the equilibrium to start from,
//     represented by the state of the tableaus, and the 'taboo' strategy, representing the variable
//     we don't need to pivot on, because we would reach an already found equilibrium (in case of
//     the first level of recursion, the artificial equilibrium).
//   */

//   for (pivot = 1; pivot <= dim1 + dim2; pivot++) {
//     if (pivot != taboo) {
//       equilibrium* eq = lemke_howson_gen(tableaus, bimatrix, dim1, dim2, pivot);

//       /*
//         If we did not reach neither an artificial equilibrium (we don't want to keep the
//         artificial equilibrium in our list of equilibria, and we have a NULL pointer
//         represantation of it), nor an already known one, we call the algorithm recursively,
//         giving the current tableaus (modified by the execution of LH) as a starting point, and
//         the strategy we just pivoted on as taboo strategy. This way we avoid a useless execution
//         of LH.
//       */

//       if (!is_artificial(eq)) {
//         lista = search_add_equilibrium(lista, eq, &found);
//         if (!found)
//           lista = all_lemke_gen(tableaus, bimatrix, dim1, dim2, pivot, lista);
//         else
//           free_equilibrium(eq);
//       } else
//         free_equilibrium(eq);

//       /*
//         In our implementation, it's of capital importance to have LH change the tableaus, so we
//         can continue the recursion without creating a new copy of the tableaus each time. This
//         makes us save a big amount of memory: otherwise we'd have a very tight upper bound on the
//         dimension of the bimatrix (an average execution on a 20x20 game allocates some hundreds
//         of mbytes of memory). Obviously, we have to restore the tableaus at their previous state.

//         We do this in a very naive way: by executing the Lemke-Howson algorithm another time with
//         the same strategy as pivot. Being LH a complementary pivoting algorithm, we will reach
//         the same equilibrium we started from, and therefore the same tableaus situation.

//         Another (definitely more performing) way to restore the previous tableau is to solve a
//         linear programming problem, knowing the variables in basis (because we know the
//         equilibrium we started from). We chose to have a slower implementation rather than
//         creating a whole linear programming library to support this algorithm, or to depend on
//         external LP libraries.
//       */
//       lemke_howson_gen(tableaus, bimatrix, dim1, dim2, pivot);
//     }
//   }

//   return lista;
// }
