#ifndef ITERATIVE_SOLVER_2D_H_
#define ITERATIVE_SOLVER_2D_H_

#include "global_float_precision.h"
#include "MPI_module/mpi_info_struct.h"

void iterative_solver_2D(FLOAT_P **rhs, FLOAT_P **final_solution, FLOAT_P **initial_guess, FLOAT_P *r, int nz, int nz_ghost, int ny, FLOAT_P dz, FLOAT_P dy, struct MpiInfo *mpi_info, FLOAT_P lower_pressure_boundary, FLOAT_P upper_pressure_boundary);
void jacobi_2D(FLOAT_P **rhs, FLOAT_P **current_solution, FLOAT_P **previous_solution, int nz, int nz_ghost, int ny, FLOAT_P dz, FLOAT_P dy, struct MpiInfo *mpi_info);
void jacobi_2D_spherical(FLOAT_P **rhs, FLOAT_P **current_solution, FLOAT_P **previous_solution, FLOAT_P *r_array, int nz, int nz_ghost, int ny, FLOAT_P dz, FLOAT_P dy, struct MpiInfo *mpi_info);
void gauss_seidel_2D(FLOAT_P **rhs, FLOAT_P **current_solution, FLOAT_P **previous_solution, int nz, int nz_ghost, int ny, FLOAT_P dz, FLOAT_P dy, struct MpiInfo *mpi_info);

#endif // ITERATIVE_SOLVER_2D_H_