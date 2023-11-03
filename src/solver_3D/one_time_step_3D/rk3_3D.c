#include "one_time_step_3D.h"

FLOAT_P rk3_3D(struct BackgroundVariables *bg, struct ForegroundVariables3D *fg_prev, struct ForegroundVariables3D *fg, struct GridInfo3D *grid_info, struct MpiInfo *mpi_info, FLOAT_P dt_last, bool first_timestep)
{
    /*
    Calculates the foreground at the next timestep using the RK3 method.

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

    // Getting grid info
    int nx = grid_info->nx;
    int ny = grid_info->ny;
    int nz_ghost = grid_info->nz_ghost;
    int nz_full = grid_info->nz_full;

    // Calculating damping factor
    FLOAT_P damping_factor[nz_full];
    calculate_damping_3D(damping_factor, bg, grid_info);

    // Calculating dt
    FLOAT_P dt = get_dt_3D(fg_prev, grid_info, dt_last, first_timestep);

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
    FLOAT_P ***k1_s1, ***k2_s1, ***k3_s1;
    FLOAT_P ***k1_vx, ***k2_vx, ***k3_vx;
    FLOAT_P ***k1_vy, ***k2_vy, ***k3_vy;
    FLOAT_P ***k1_vz, ***k2_vz, ***k3_vz;

    // Allocate memory for k1, k2, k3
    allocate_3D_array(&k1_s1, nz_full, ny, nx);
    allocate_3D_array(&k2_s1, nz_full, ny, nx);
    allocate_3D_array(&k3_s1, nz_full, ny, nx);
    allocate_3D_array(&k1_vx, nz_full, ny, nx);
    allocate_3D_array(&k2_vx, nz_full, ny, nx);
    allocate_3D_array(&k3_vx, nz_full, ny, nx);
    allocate_3D_array(&k1_vy, nz_full, ny, nx);
    allocate_3D_array(&k2_vy, nz_full, ny, nx);
    allocate_3D_array(&k3_vy, nz_full, ny, nx);
    allocate_3D_array(&k1_vz, nz_full, ny, nx);
    allocate_3D_array(&k2_vz, nz_full, ny, nx);
    allocate_3D_array(&k3_vz, nz_full, ny, nx);

    // Inside the grid
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                // Calculating k1 for the grid
                k1_s1[i][j][k] = rhs_ds1_dt_3D(bg, fg_prev, grid_info, i, j, k);
                k1_vx[i][j][k] = rhs_dvx_dt_3D(bg, fg_prev, grid_info, i, j, k);
                k1_vy[i][j][k] = rhs_dvy_dt_3D(bg, fg_prev, grid_info, i, j, k);
                k1_vz[i][j][k] = rhs_dvz_dt_3D(bg, fg_prev, grid_info, i, j, k);
            }
        }
    }

    // Bottom boundary k1
    if (!mpi_info->has_neighbor_below) // If process is at the bottom
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
    if (mpi_info->has_neighbor_above) // If process is at the top
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
                    fg->s1[i][j][k] = fg_prev->s1[i][j][k] + dt/2.0 * k1_s1[i][j][k];
                    fg->vx[i][j][k] = fg_prev->vx[i][j][k] + dt/2.0 * k1_vx[i][j][k];
                    fg->vy[i][j][k] = fg_prev->vy[i][j][k] + dt/2.0 * k1_vy[i][j][k];
                    fg->vz[i][j][k] = fg_prev->vz[i][j][k] + dt/2.0 * k1_vz[i][j][k];
            }
        }
    }

    // Extrapolatating s1, vy and vz to ghost cells
    extrapolate_3D_array_down(fg->s1, nz_ghost, ny, nx); // Extrapolating s1 to ghost cells below
    extrapolate_3D_array_up(fg->s1, nz_full, nz_ghost, ny, nx); // Extrapolating s1 to ghost cells above
    extrapolate_3D_array_down(fg->vx, nz_ghost, ny, nx); // Extrapolating vx to ghost cells below
    extrapolate_3D_array_up(fg->vx, nz_full, nz_ghost, ny, nx); // Extrapolating vx to ghost cells above
    extrapolate_3D_array_down(fg->vy, nz_ghost, ny, nx); // Extrapolating vy to ghost cells below
    extrapolate_3D_array_up(fg->vy, nz_full, nz_ghost, ny, nx); // Extrapolating vy to ghost cells above
    extrapolate_3D_array_down(fg->vz, nz_ghost, ny, nx); // Extrapolating vz to ghost cells below
    extrapolate_3D_array_up(fg->vz, nz_full, nz_ghost, ny, nx); // Extrapolating vz to ghost cells above

    // Solving elliptic equation
    solve_elliptic_equation_3D(bg, fg, fg, grid_info, mpi_info); // Getting p1
    
    // Extrapolating p1 to ghost cells
    extrapolate_3D_array_down(fg->p1, nz_ghost, ny, nx); // Extrapolating p1 to ghost cells below
    extrapolate_3D_array_up(fg->p1, nz_full, nz_ghost, ny, nx); // Extrapolating p1 to ghost cells above

    // Updating mid-calculation variables
    first_law_thermodynamics_3D(fg, bg, grid_info);
    equation_of_state_3D(fg, bg, grid_info);

    #if MPI_ON == 1
        // Communicate ghost cells
    #endif // MPI_ON

    // Calculating k2 for the grid
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                k2_s1[i][j][k] = rhs_ds1_dt_3D(bg, fg, grid_info, i, j, k);
                k2_vx[i][j][k] = rhs_dvx_dt_3D(bg, fg, grid_info, i, j, k);
                k2_vy[i][j][k] = rhs_dvy_dt_3D(bg, fg, grid_info, i, j, k);
                k2_vz[i][j][k] = rhs_dvz_dt_3D(bg, fg, grid_info, i, j, k);
            }
        }
    }

    // Bottom boundary k2
    if (!mpi_info->has_neighbor_below) // If process is at the bottom of the grid
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
    if (!mpi_info->has_neighbor_above) // If process is at the top of the grid
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

    // Updating fg to hold mid-calculation variables
    for (int i = nz_ghost; i < nz_full-nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                fg->s1[i][j][k] = fg_prev->s1[i][j][k] - dt * k1_s1[i][j][k] + 2.0 * dt * k2_s1[i][j][k];
                fg->vx[i][j][k] = fg_prev->vx[i][j][k] - dt * k1_vx[i][j][k] + 2.0 * dt * k2_vx[i][j][k];
                fg->vy[i][j][k] = fg_prev->vy[i][j][k] - dt * k1_vy[i][j][k] + 2.0 * dt * k2_vy[i][j][k];
                fg->vz[i][j][k] = fg_prev->vz[i][j][k] - dt * k1_vz[i][j][k] + 2.0 * dt * k2_vz[i][j][k];
            }
        }
    }

    // Extrapolatating s1, vy, vx and vz to ghost cells
    extrapolate_3D_array_down(fg->s1, nz_ghost, ny, nx); // Extrapolating s1 to ghost cells below
    extrapolate_3D_array_up(fg->s1, nz_full, nz_ghost, ny, nx); // Extrapolating s1 to ghost cells above
    extrapolate_3D_array_down(fg->vx, nz_ghost, ny, nx); // Extrapolating vx to ghost cells below
    extrapolate_3D_array_up(fg->vx, nz_full, nz_ghost, ny, nx); // Extrapolating vx to ghost cells above
    extrapolate_3D_array_down(fg->vy, nz_ghost, ny, nx); // Extrapolating vy to ghost cells below
    extrapolate_3D_array_up(fg->vy, nz_full, nz_ghost, ny, nx); // Extrapolating vy to ghost cells above
    extrapolate_3D_array_down(fg->vz, nz_ghost, ny, nx); // Extrapolating vz to ghost cells below
    extrapolate_3D_array_up(fg->vz, nz_full, nz_ghost, ny, nx); // Extrapolating vz to ghost cells above

    // Solving elliptic equation
    solve_elliptic_equation_3D(bg, fg, fg, grid_info, mpi_info); // Getting p1
    
    // Extrapolating p1 to ghost cells
    extrapolate_3D_array_down(fg->p1, nz_ghost, ny, nx); // Extrapolating p1 to ghost cells below
    extrapolate_3D_array_up(fg->p1, nz_full, nz_ghost, ny, nx); // Extrapolating p1 to ghost cells above

    // Updating mid-calculation variables
    first_law_thermodynamics_3D(fg, bg, grid_info);
    equation_of_state_3D(fg, bg, grid_info);

    #if MPI_ON == 1
        // Communicate ghost cells
    #endif // MPI_ON

    // Calculating k3 for the grid
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                k3_s1[i][j][k] = rhs_ds1_dt_3D(bg, fg, grid_info, i, j, k);
                k3_vx[i][j][k] = rhs_dvx_dt_3D(bg, fg, grid_info, i, j, k);
                k3_vy[i][j][k] = rhs_dvy_dt_3D(bg, fg, grid_info, i, j, k);
                k3_vz[i][j][k] = rhs_dvz_dt_3D(bg, fg, grid_info, i, j, k);
            }
        }
    }

    // Bottom boundary k3
    if (!mpi_info->has_neighbor_below) // If process is at the bottom of the grid
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                k3_vx[nz_ghost][j][k] = rhs_dvx_dt_3D_vertical_boundary(bg, fg, grid_info, nz_ghost, j, k);
                k3_vy[nz_ghost][j][k] = rhs_dvy_dt_3D_vertical_boundary(bg, fg, grid_info, nz_ghost, j, k);
            }
        }
    }

    // Top boundary k3
    if (!mpi_info->has_neighbor_above) // If process is at the top of the grid
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                k3_vx[nz_full-nz_ghost-1][j][k] = rhs_dvx_dt_3D_vertical_boundary(bg, fg, grid_info, nz_full-nz_ghost-1, j, k);
                k3_vy[nz_full-nz_ghost-1][j][k] = rhs_dvy_dt_3D_vertical_boundary(bg, fg, grid_info, nz_full-nz_ghost-1, j, k);
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
                fg->s1[i][j][k] = fg_prev->s1[i][j][k] + dt/6.0 * (k1_s1[i][j][k] + 4.0 * k2_s1[i][j][k] + k3_s1[i][j][k]);
                fg->vx[i][j][k] = fg_prev->vx[i][j][k] + dt/6.0 * (k1_vx[i][j][k] + 4.0 * k2_vx[i][j][k] + k3_vx[i][j][k]);
                fg->vy[i][j][k] = fg_prev->vy[i][j][k] + dt/6.0 * (k1_vy[i][j][k] + 4.0 * k2_vy[i][j][k] + k3_vy[i][j][k]);
                fg->vz[i][j][k] = fg_prev->vz[i][j][k] + dt/6.0 * (k1_vz[i][j][k] + 4.0 * k2_vz[i][j][k] + k3_vz[i][j][k]);
            }
        }
    }

    // Apply boundary
    for (int i = nz_ghost; i < nz_full-nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int k = 0; k < nx; k++)
            {
                fg->s1[i][j][k] *= damping_factor[i];
                fg->vz[i][j][k] *= damping_factor[i];
            }
        }
    }

    // Extrapolating s1, vx, vy, vz
    extrapolate_3D_array_down(fg->s1, nz_ghost, ny, nx); // Extrapolating s1 to ghost cells below
    extrapolate_3D_array_up(fg->s1, nz_full, nz_ghost, ny, nx); // Extrapolating s1 to ghost cells above
    extrapolate_3D_array_down(fg->vx, nz_ghost, ny, nx); // Extrapolating vx to ghost cells below
    extrapolate_3D_array_up(fg->vx, nz_full, nz_ghost, ny, nx); // Extrapolating vx to ghost cells above
    extrapolate_3D_array_down(fg->vy, nz_ghost, ny, nx); // Extrapolating vy to ghost cells below
    extrapolate_3D_array_up(fg->vy, nz_full, nz_ghost, ny, nx); // Extrapolating vy to ghost cells above
    extrapolate_3D_array_down(fg->vz, nz_ghost, ny, nx); // Extrapolating vz to ghost cells below
    extrapolate_3D_array_up(fg->vz, nz_full, nz_ghost, ny, nx); // Extrapolating vz to ghost cells above

    // Solving elliptic equation
    solve_elliptic_equation_3D(bg, fg_prev, fg, grid_info, mpi_info); // Getting p1
    
    // Extrapolating p1 to ghost cells
    extrapolate_3D_array_down(fg->p1, nz_ghost, ny, nx); // Extrapolating p1 to ghost cells below
    extrapolate_3D_array_up(fg->p1, nz_full, nz_ghost, ny, nx); // Extrapolating p1 to ghost cells above


    // Solving equation of state to get rho1 and first law of thermodynamics to get T1
    first_law_thermodynamics_3D(fg, bg, grid_info);
    equation_of_state_3D(fg, bg, grid_info);

    // Deallocating memory
    deallocate_3D_array(k1_s1);
    deallocate_3D_array(k2_s1);
    deallocate_3D_array(k3_s1);
    deallocate_3D_array(k1_vx);
    deallocate_3D_array(k2_vx);
    deallocate_3D_array(k3_vx);
    deallocate_3D_array(k1_vy);
    deallocate_3D_array(k2_vy);
    deallocate_3D_array(k3_vy);
    deallocate_3D_array(k1_vz);
    deallocate_3D_array(k2_vz);
    deallocate_3D_array(k3_vz);     

    return dt;       
}