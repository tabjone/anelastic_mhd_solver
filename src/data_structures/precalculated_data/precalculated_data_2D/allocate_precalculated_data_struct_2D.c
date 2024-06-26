#include <stdlib.h>
#include "global_float_precision.h"
#include "array_utilities/array_memory_management/array_memory_management.h"
#include "precalculated_data_struct_2D.h"

void allocate_precalculate_data_struct_2D(struct PrecalculatedVariables2D **pv, int nz_full, int ny)
{
    // Allocate memory for the struct
    *pv = (struct PrecalculatedVariables2D *)malloc(sizeof(struct PrecalculatedVariables2D));
    if (*pv == NULL) {
    // Handle error, for example, by returning an error code or exiting.
    fprintf(stderr, "Failed to allocate memory for PrecalculatedVariables2D struct\n");
    exit(EXIT_FAILURE);
    }

    // Allocate memory for the arrays
    allocate_1D_array(&(*pv)->one_over_rho0, nz_full);
    allocate_1D_array(&(*pv)->grad_g, nz_full);
    allocate_1D_array(&(*pv)->grad_rho0, nz_full);
    allocate_1D_array(&(*pv)->eta_over_four_pi_rho0_T0, nz_full);
    allocate_1D_array(&(*pv)->grad_T0, nz_full);
    allocate_1D_array(&(*pv)->VIS_COEFF_over_rho0, nz_full);
    allocate_1D_array(&(*pv)->VIS_COEFF_over_T0_rho0, nz_full);
    allocate_1D_array(&(*pv)->THERM_COEFF_over_T0_rho0, nz_full);
    allocate_1D_array(&(*pv)->damping_factor, nz_full);
    allocate_1D_array(&(*pv)->vz_horizontal_average, nz_full);
    allocate_1D_array(&(*pv)->vy_vertical_average, ny);
}