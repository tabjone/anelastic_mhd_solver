#include "global_float_precision.h"
#include "global_parameters.h"
#include "grid_info_struct_2D.h"
#include "global_constants.h"
#include "MPI_module/mpi_info_struct.h"
#include <math.h>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#include <stdio.h>

void initialize_grid_info_2D(struct GridInfo2D *grid_info, struct MpiInfo *mpi_info)
{
    printf("hello from initialize_grid_info_2D\n");
    // Calculating the size of the grid
    FLOAT_P L_z = (R_END - R_START)*R_SUN;
    #if COORDINATES == 0
        FLOAT_P L_y = Y_SIZE*R_SUN;
    #elif COORDINATES == 1
        FLOAT_P L_y = 2.0*M_PI*Y_SIZE;
    #endif

    // Calculating the size of the grid cells
    FLOAT_P dy = L_y/(NY);
    FLOAT_P dz = L_z/(NZ - 1);

    // Calculating the number of ghost cells
    int nz_ghost;
    if (UPWIND_ORDER >= (CENTRAL_ORDER-2))
    {
        nz_ghost = UPWIND_ORDER;
    }
    else
    {
        nz_ghost = CENTRAL_ORDER - 2;
    }

    // Number of cells to be divided among processes
    int total_cells = NZ;

    // Average number of cells per process
    int avg_cells_per_process = total_cells / mpi_info->size;

    // Extra cells that can't be evenly distributed among processes
    int extra_cells = total_cells % mpi_info->size;

    int my_offset, my_nz;

    // If current rank is less than extra_cells, then it takes one of the extra cells
    if (mpi_info->rank < extra_cells) {
        my_offset = mpi_info->rank * (avg_cells_per_process + 1);
        my_nz = avg_cells_per_process + 1;
    } else {
        my_offset = mpi_info->rank * avg_cells_per_process + extra_cells;
        my_nz = avg_cells_per_process;
    }

    FLOAT_P z_offset = (my_offset) * dz;

    FLOAT_P z0, z1, y0, y1;

    z0 = R_SUN * R_START + (my_offset) * dz;
    z1 = R_SUN * R_START + (my_offset + my_nz-1) * dz;
    y0 = 0.0;
    #if COORDINATES == 0
        y1 = R_SUN * Y_SIZE;
    #elif COORDINATES == 1
        y1 = 2.0 * M_PI * Y_SIZE;
    #endif

    int nz_full = my_nz + 2*nz_ghost;

    printf("Process: %d, z0=%.2f", mpi_info->rank, z0/R_SUN);

    grid_info->z0 = z0;
    grid_info->z1 = z1;
    grid_info->y0 = y0;
    grid_info->y1 = y1;
    grid_info->nz = my_nz;
    grid_info->ny = NY;
    grid_info->nz_ghost = nz_ghost;
    grid_info->nz_full = nz_full;
    grid_info->dy = dy;
    grid_info->dz = dz;
    grid_info->z_offset = z_offset;
}