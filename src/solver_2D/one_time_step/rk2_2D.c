#include "one_time_step.h"

FLOAT_P rk2_2D(struct BackgroundVariables *bg, struct ForegroundVariables2D *fg_prev, struct ForegroundVariables2D *fg, struct GridInfo2D *grid_info, struct MpiInfo *mpi_info, FLOAT_P dt_last, bool first_timestep)
{
    /*
    Calculates the foreground at the next timestep using the RK2 method.

    Parameters
    ----------
    bg : struct
        A pointer to the BackgroundVariables struct.
    fg_prev : struct
        A pointer to the ForegroundVariables2D struct at the previous timestep.
    fg : struct
        A pointer to the ForegroundVariables2D struct at the current timestep.
    grid_info : struct
        A pointer to the GridInfo2D struct.
    mpi_info : struct
        A pointer to the MpiInfo struct.
    dt_last : FLOAT_P
        The timestep used at the previous timestep.
    first_timestep : bool
        True if this is the first timestep, false otherwise.
    */

    // Getting grid info
    int nz_ghost = grid_info->nz_ghost;
    int nz_full = grid_info->nz_full;
    int ny = grid_info->ny;

    // Calculating dt
    FLOAT_P dt = get_dt(fg_prev, grid_info, dt_last, first_timestep);

    #if MPI_ON == 1
        FLOAT_P reduced_dt;
        // Picking smallest dt from all processes
        MPI_Allreduce(&dt, &reduced_dt, 1, MPI_FLOAT_P, MPI_MIN, MPI_COMM_WORLD);
        dt = reduced_dt;
    #endif // MPI_ON

    // Slopes
    FLOAT_P **k1_s1, **k2_s1;
    FLOAT_P **k1_vy, **k2_vy;
    FLOAT_P **k1_vz, **k2_vz;

    // Allocate memory for k1, k2
    allocate_2D_array(&k1_s1, nz_full, ny);
    allocate_2D_array(&k2_s1, nz_full, ny);
    allocate_2D_array(&k1_vy, nz_full, ny);
    allocate_2D_array(&k2_vy, nz_full, ny);
    allocate_2D_array(&k1_vz, nz_full, ny);
    allocate_2D_array(&k2_vz, nz_full, ny);

    // Inside the grid
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            // Calculating k1 for the grid
            k1_s1[i][j] = rhs_ds1_dt_2D(bg, fg_prev, grid_info, i, j);
            k1_vy[i][j] = rhs_dvy_dt_2D(bg, fg_prev, grid_info, i, j);
            k1_vz[i][j] = rhs_dvz_dt_2D(bg, fg_prev, grid_info, i, j);
        }
    }

    // Updating fg to hold mid-calculation variables
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            fg->s1[i][j] = fg_prev->s1[i][j] + dt * k1_s1[i][j];
            fg->vy[i][j] = fg_prev->vy[i][j] + dt * k1_vy[i][j];
            fg->vz[i][j] = fg_prev->vz[i][j] + dt * k1_vz[i][j];
        }
    }

    apply_vertical_boundary_damping(fg, bg, grid_info, mpi_info, dt);

    update_vertical_boundary_ghostcells_2D(fg->s1, grid_info, mpi_info);
    update_vertical_boundary_ghostcells_2D(fg->vy, grid_info, mpi_info);
    update_vertical_boundary_ghostcells_2D(fg->vz, grid_info, mpi_info);

    // Updating mid calculation variables
    first_law_thermodynamics(fg, bg, grid_info);
    equation_of_state(fg, bg, grid_info);

    // Calculating p1
    solve_elliptic_equation(bg, fg, fg, grid_info, mpi_info);
    update_vertical_boundary_ghostcells_2D(fg->p1, grid_info, mpi_info);

    // Calculating k2
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            // Calculating k2 for the grid
            k2_s1[i][j] = rhs_ds1_dt_2D(bg, fg, grid_info, i, j);
            k2_vy[i][j] = rhs_dvy_dt_2D(bg, fg, grid_info, i, j);
            k2_vz[i][j] = rhs_dvz_dt_2D(bg, fg, grid_info, i, j);
        }
    }

    // Updating variables for grid and boundaries
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            fg->s1[i][j] = fg_prev->s1[i][j] + dt/2.0 * (k1_s1[i][j] + k2_s1[i][j]);
            fg->vy[i][j] = fg_prev->vy[i][j] + dt/2.0 * (k1_vy[i][j] + k2_vy[i][j]);
            fg->vz[i][j] = fg_prev->vz[i][j] + dt/2.0 * (k1_vz[i][j] + k2_vz[i][j]);
        }
    }

    apply_vertical_boundary_damping(fg, bg, grid_info, mpi_info, dt);

    update_vertical_boundary_ghostcells_2D(fg->s1, grid_info, mpi_info);
    update_vertical_boundary_ghostcells_2D(fg->vy, grid_info, mpi_info);
    update_vertical_boundary_ghostcells_2D(fg->vz, grid_info, mpi_info);

    // Solving algebraic equations
    first_law_thermodynamics(fg, bg, grid_info);
    equation_of_state(fg, bg, grid_info);

    // Calculating p1
    solve_elliptic_equation(bg, fg, fg, grid_info, mpi_info);
    update_vertical_boundary_ghostcells_2D(fg->p1, grid_info, mpi_info);

    // Deallocating memory
    deallocate_2D_array(k1_s1);
    deallocate_2D_array(k2_s1);
    deallocate_2D_array(k1_vy);
    deallocate_2D_array(k2_vy);
    deallocate_2D_array(k1_vz);
    deallocate_2D_array(k2_vz);

    return dt;
}