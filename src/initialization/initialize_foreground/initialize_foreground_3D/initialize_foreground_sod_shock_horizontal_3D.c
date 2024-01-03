#include "data_structures/grid_info/grid_info_3D/grid_info_struct_3D.h"
#include "data_structures/foreground_data/foreground_data_3D/foreground_variables_struct_3D.h"
#include "data_structures/background_data/background_variables_struct.h"
#include "MPI_module/MPI_module.h"
#include "solver/equation_of_state/equation_of_state_3D/equation_of_state_3D.h"
#include "global_initialization.h"
#include "initialize_foreground_3D.h"
#include "global_constants.h"
#include "global_parameters.h"

void initialize_foreground_sod_shock_horizontal_3D(struct ForegroundVariables3D *fg, struct BackgroundVariables *bg, struct GridInfo3D *grid_info, struct MpiInfo *mpi_info)
{
    /*
    Initializes the foreground struct with a Sod Shock Tube test.

    Parameters
    ----------
    fg : ForegroundVariables3D
        A pointer to the ForegroundVariables3D struct.
    bg : BackgroundVariables
        A pointer to the BackgroundVariables struct.
    grid_info : GridInfo3D
        A pointer to the GridInfo3D struct.
    */

    // Getting grid info
    int nz_full = grid_info->nz_full;
    int nz_ghost = grid_info->nz_ghost;
    int nz = grid_info->nz;
    int ny = grid_info->ny;
    int nx = grid_info->nx;

    FLOAT_P left_rho, left_p;
    if (mpi_info->rank == 0)
    {
        printf("Initializing Sod Shock Tube\n");
        // For now just letting the left side be one of the top values from process 1
        left_rho = bg->rho0[nz] * 1.0e-8;
        left_p = bg->p0[nz] * 1.0e-8;
    }
    MPI_Bcast(&left_rho, 1, MPI_FLOAT_P, 0, MPI_COMM_WORLD);
    MPI_Bcast(&left_p, 1, MPI_FLOAT_P, 0, MPI_COMM_WORLD);

    FLOAT_P right_rho = left_rho * 0.5;
    FLOAT_P right_p = left_p * 0.5;

    FLOAT_P c_p = K_B / (MU * M_U) / (1.0 - 1.0/GAMMA);

    initialize_foreground_zeros_3D(fg, grid_info);

    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                if (j < ny/2)
                {
                    fg->p1[i][j][k] = left_p;
                    fg->rho1[i][j][k] = left_rho;
                }
                else
                {
                    fg->p1[i][j][k] = right_p;
                    fg->rho1[i][j][k] = right_rho;
                }
                // Getting T1 from equation of state
                fg->T1[i][j][k] = bg->T0[i] * (fg->p1[i][j][k]/bg->p0[i] - fg->rho1[i][j][k]/bg->rho0[i]);
                // Getting entropy from first law of thermodynamics
                fg->s1[i][j][k] = c_p * (fg->T1[i][j][k]/bg->T0[i] - fg->p1[i][j][k]/bg->p0[i]);
            }
        }
    }

    communicate_3D_ghost_above_below(fg->p1, mpi_info, nz, nz_ghost, ny, nx);
    communicate_3D_ghost_above_below(fg->rho1, mpi_info, nz, nz_ghost, ny, nx);
    communicate_3D_ghost_above_below(fg->T1, mpi_info, nz, nz_ghost, ny, nx);
    communicate_3D_ghost_above_below(fg->s1, mpi_info, nz, nz_ghost, ny, nx);
    communicate_3D_ghost_above_below(fg->vy, mpi_info, nz, nz_ghost, ny, nx);
    communicate_3D_ghost_above_below(fg->vz, mpi_info, nz, nz_ghost, ny, nx);
}