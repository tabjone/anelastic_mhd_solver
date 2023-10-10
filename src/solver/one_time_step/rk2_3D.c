#include "one_time_step.h"

#if DIMENSIONS == 3
FLOAT_P rk2_3D(struct BackgroundVariables *bg, struct ForegroundVariables *fg_prev, struct ForegroundVariables *fg, struct GridInfo *grid_info, struct MpiInfo *mpi_info, FLOAT_P dt_last, bool first_timestep)
{
    /*
    Calculates the foreground at the next timestep using the RK2 method.

    Parameters
    ----------
    bg : struct
        A pointer to the BackgroundVariables struct.
    fg_prev : struct
        A pointer to the ForegroundVariables struct at the previous timestep.
    fg : struct
        A pointer to the ForegroundVariables struct at the current timestep.
    grid_info : struct
        A pointer to the GridInfo struct.
    mpi_info : struct
        A pointer to the MpiInfo struct.
    dt_last : FLOAT_P
        The timestep used at the previous timestep.
    first_timestep : bool
        True if this is the first timestep, false otherwise.
    */

    // Solving elliptic equation
    solve_elliptic_equation(bg, fg_prev, fg, grid_info); // Getting p1

    // Extrapolating p1 to ghost cells
    extrapolate_3D_array_down(fg->p1, grid_info); // Extrapolating p1 to ghost cells below
    extrapolate_3D_array_up(fg->p1, grid_info); // Extrapolating p1 to ghost cells above

    #if MPI_ON == 1
        // Communicate ghost cells
    #endif // MPI_ON

    // Getting grid info
    int nz_ghost = grid_info->nz_ghost;
    int nz_full = grid_info->nz_full;
    int ny = grid_info->ny;
    int nx = grid_info->nx;

    // Calculating damping factor
    FLOAT_P damping_factor[nz_full];
    calculate_damping(damping_factor, grid_info, mpi_info);

    // Calculating dt
    FLOAT_P dt = get_dt(fg_prev, grid_info, dt_last, first_timestep);

    #if MPI_ON == 1
        // Picking smallest dt from all processes
        MPI_Allreduce(&dt, &dt, 1, MPI_FLOAT_P, MPI_MIN, MPI_COMM_WORLD);
    #endif // MPI_ON

    // Using the fg struct to store mid-calculation variables. Filling these with fg_prev values.
    for (int i = 0; i < nz_full; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                fg->T1[i][j][k] = fg_prev->T1[i][j][k];
                fg->rho1[i][j][k] = fg_prev->rho1[i][j][k];
            }
        }
    }

    // Slopes
    FLOAT_P ***k1_s1, ***k2_s1;
    FLOAT_P ***k1_vx, ***k2_vx;
    FLOAT_P ***k1_vy, ***k2_vy;
    FLOAT_P ***k1_vz, ***k2_vz;

    // Allocating memory for slopes
    allocate_3D_array(&k1_s1, nz_full, ny, nx);
    allocate_3D_array(&k2_s1, nz_full, ny, nx);
    allocate_3D_array(&k1_vx, nz_full, ny, nx);
    allocate_3D_array(&k2_vx, nz_full, ny, nx);
    allocate_3D_array(&k1_vy, nz_full, ny, nx);
    allocate_3D_array(&k2_vy, nz_full, ny, nx);
    allocate_3D_array(&k1_vz, nz_full, ny, nx);
    allocate_3D_array(&k2_vz, nz_full, ny, nx);

    // Inside the grid
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                // Calculating k1 for the grid
                k1_s1[i][j][k] = damping_factor[i]*rhs_ds1_dt_3D(bg, fg_prev, grid_info, i, j, k);
                k1_vx[i][j][k] = rhs_dvx_dt_3D(bg, fg_prev, grid_info, i, j, k);
                k1_vy[i][j][k] = rhs_dvy_dt_3D(bg, fg_prev, grid_info, i, j, k);
                k1_vz[i][j][k] = damping_factor[i]*rhs_dvz_dt_3D(bg, fg_prev, grid_info, i, j, k);
            }
        }
    }

    // Bottom boundary k1
    if (!mpi_info->has_neighbor_below) // If this process is at the bottom of the grid
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                k1_vx[nz_ghost][j][k] = rhs_dvx_dt_3D_vertical_boundary(bg, fg_prev, grid_info, nz_ghost, j, k);
                k1_vy[nz_ghost][j][k] = rhs_dvy_dt_3D_vertical_boundary(bg, fg_prev, grid_info, nz_ghost, j, k);
            }
        }
    }

    // Top boundary k1
    if (!mpi_info->has_neighbor_above) // If this process is at the top of the grid
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                k1_vx[nz_full-nz_ghost-1][j][k] = rhs_dvx_dt_3D_vertical_boundary(bg, fg_prev, grid_info, nz_full-nz_ghost-1, j, k);
                k1_vy[nz_full-nz_ghost-1][j][k] = rhs_dvy_dt_3D_vertical_boundary(bg, fg_prev, grid_info, nz_full-nz_ghost-1, j, k);
            }
        }
    }

    // Updating fg to hold mid-calculation variables
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                fg->s1[i][j][k] = fg_prev->s1[i][j][k] + dt * k1_s1[i][j][k];
                fg->vx[i][j][k] = fg_prev->vx[i][j][k] + dt * k1_vx[i][j][k];
                fg->vy[i][j][k] = fg_prev->vy[i][j][k] + dt * k1_vy[i][j][k];
                fg->vz[i][j][k] = fg_prev->vz[i][j][k] + dt * k1_vz[i][j][k];
            }
        }
    }

    // Extrapolating mid-calculation variables
    extrapolate_3D_array_up(fg->s1, grid_info);
    extrapolate_3D_array_down(fg->s1, grid_info);
    extrapolate_3D_array_up(fg->vx, grid_info);
    extrapolate_3D_array_down(fg->vx, grid_info);
    extrapolate_3D_array_up(fg->vy, grid_info);
    extrapolate_3D_array_down(fg->vy, grid_info);
    extrapolate_3D_array_up(fg->vz, grid_info);
    extrapolate_3D_array_down(fg->vz, grid_info);

    #if MPI_ON == 1
        // Communicate ghost cells
    #endif // MPI_ON

    // Calculating k2
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                k2_s1[i][j][k] = damping_factor[i]*rhs_ds1_dt_3D(bg, fg, grid_info, i, j, k);
                k2_vx[i][j][k] = rhs_dvx_dt_3D(bg, fg, grid_info, i, j, k);
                k2_vy[i][j][k] = rhs_dvy_dt_3D(bg, fg, grid_info, i, j, k);
                k2_vz[i][j][k] = damping_factor[i]*rhs_dvz_dt_3D(bg, fg, grid_info, i, j, k);
            }
        }
    }

    // Bottom boundary k2
    if (!mpi_info->has_neighbor_below) // If this process is at the bottom of the grid
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                k2_vx[nz_ghost][j][k] = rhs_dvx_dt_3D_vertical_boundary(bg, fg, grid_info, nz_ghost, j, k);
                k2_vy[nz_ghost][j][k] = rhs_dvy_dt_3D_vertical_boundary(bg, fg, grid_info, nz_ghost, j, k);
            }
        }
    }

    // Top boundary k2
    if (!mpi_info->has_neighbor_above) // If this process is at the top of the grid
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                k2_vx[nz_full-nz_ghost-1][j][k] = rhs_dvx_dt_3D_vertical_boundary(bg, fg, grid_info, nz_full-nz_ghost-1, j, k);
                k2_vy[nz_full-nz_ghost-1][j][k] = rhs_dvy_dt_3D_vertical_boundary(bg, fg, grid_info, nz_full-nz_ghost-1, j, k);
            }
        }
    }

    // Updating variables
    for (int i = nz_ghost; i < nz_full-nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                fg->s1[i][j][k] = fg_prev->s1[i][j][k] + dt/2.0 * (k1_s1[i][j][k] + k2_s1[i][j][k]);
                fg->vx[i][j][k] = fg_prev->vx[i][j][k] + dt/2.0 * (k1_vx[i][j][k] + k2_vx[i][j][k]);
                fg->vy[i][j][k] = fg_prev->vy[i][j][k] + dt/2.0 * (k1_vy[i][j][k] + k2_vy[i][j][k]);
                fg->vz[i][j][k] = fg_prev->vz[i][j][k] + dt/2.0 * (k1_vz[i][j][k] + k2_vz[i][j][k]);
            }
        }
    }

    // Extrapolating variables
    extrapolate_3D_array_up(fg->s1, grid_info);
    extrapolate_3D_array_down(fg->s1, grid_info);
    extrapolate_3D_array_up(fg->vx, grid_info);
    extrapolate_3D_array_down(fg->vx, grid_info);
    extrapolate_3D_array_up(fg->vy, grid_info);
    extrapolate_3D_array_down(fg->vy, grid_info);
    extrapolate_3D_array_up(fg->vz, grid_info);
    extrapolate_3D_array_down(fg->vz, grid_info);

    #if MPI_ON == 1
        // Communicate ghost cells
    #endif // MPI_ON

    // Freeing memory
    deallocate_3D_array(k1_s1);
    deallocate_3D_array(k2_s1);
    deallocate_3D_array(k1_vx);
    deallocate_3D_array(k2_vx);
    deallocate_3D_array(k1_vy);
    deallocate_3D_array(k2_vy);
    deallocate_3D_array(k1_vz);
    deallocate_3D_array(k2_vz);

    // Solving algebraic equation
    first_law_thermodynamics(fg, bg, grid_info);
    equation_of_state(fg, bg, grid_info);

    return dt;
}
#endif // DIMENSIONS == 3