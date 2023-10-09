#ifndef SOLVE_ELLIPTIC_EQUATION_H__
#define SOLVE_ELLIPTIC_EQUATION_H__

#include "shared_files.h"
#include <math.h>
#include "global_parameters.h"
#include "../rhs_functions/rhs_functions.h"

#if DIMENSIONS == 1
void gauss_seidel_1D(FLOAT_P *b, FLOAT_P *p1, FLOAT_P *initial_p1, struct GridInfo *grid_info);
#elif DIMENSIONS == 2
void gauss_seidel_2D(FLOAT_P **b, FLOAT_P **p1, FLOAT_P **initial_p1, struct GridInfo *grid_info);
#elif DIMENSIONS == 3
void gauss_seidel_3D(FLOAT_P ***b, FLOAT_P ***p1, FLOAT_P ***initial_p1, struct GridInfo *grid_info);
#endif // DIMENSIONS


void solve_elliptic_equation(struct BackgroundVariables *bg, struct ForegroundVariables *fg_prev, struct ForegroundVariables *fg, struct GridInfo *grid_info);

void gauss_seidel(FLOAT_P **A, FLOAT_P *b, FLOAT_P *x, int N, int maxIterations, FLOAT_P tolerance);

void gauss_seidel_fast_second_order(FLOAT_P **b, FLOAT_P **p1, FLOAT_P **initial_p1, struct GridInfo *grid_info);

#endif // SOLVE_ELLIPTIC_EQUATION_H__