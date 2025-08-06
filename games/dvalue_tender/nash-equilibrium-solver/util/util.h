#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "lemke-howson-killer/algorithm.h"
using std::cin, std::cout, std::endl;
using std::max, std::min;
using std::string;
using std::thread;
using std::unique_ptr, std::shared_ptr;
using std::vector, std::array, std::pair, std::make_pair;

const double NaN = std::numeric_limits<double>::signaling_NaN();

const int processor_count = std::thread::hardware_concurrency();

void bprint(int x, int begin = 8) {
  while (begin) begin--, putchar(x & (1 << begin) ? '1' : '0');
}

char *bprints(int x, int begin = 8) {
  char *result = (char *)malloc(begin + 1), *pos = result;
  while (begin) {
    begin--;
    *pos++ = (x & (1 << begin) ? '1' : '0');
  }
  *pos = '\0';
  return result;
}

double **create_bimatrix(int dim1, int dim2) {
  double **bimatrix = (double **)malloc(sizeof(double *) * 2 * dim1);
  for (int i = 0; i < (2 * dim1); i++) bimatrix[i] = (double *)malloc(sizeof(double) * dim2);
  return bimatrix;
}

// bimatrix should be all positive. positivize_bimatrix() might be useful somewhere.
double *solve_equilibrium(double **bimatrix, int dim1, int dim2) {
  auto tableaus = create_systems(bimatrix, dim1, dim2);
  auto found_equilibria = lemke_howson_gen(tableaus, bimatrix, dim1, dim2, 1);
  free_tableaus(tableaus, dim1, dim2);
  return found_equilibria;
}

double calculate_ev(double **bimatrix, int dim1, int dim2) {
  auto eq = solve_equilibrium(bimatrix, dim1, dim2);
  double sum = 0;
  for (int i = 0; i < dim1; i++) {
    for (int j = 0; j < dim2; j++) {
      sum += eq[i] * eq[j + dim1] * bimatrix[i][j];
    }
  }
  free(eq);
  return sum;
}

int readUnsigned() {
  int res = 0, c;
  while (c = getchar(), c < '0' || c > '9')
    ;
  do {
    res = res * 10 + (c - 48);
  } while (c = getchar(), c >= '0' && c <= '9');
  return res;
}

int64_t get_time_ms() {
  auto duration = std::chrono::system_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}