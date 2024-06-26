#include "data_structures/grid_info/grid_info_2D/grid_info_struct_2D.h"
#include "data_structures/foreground_data/foreground_data_2D/foreground_variables_struct_2D.h"
#include "data_structures/background_data/background_variables_struct.h"
#include "array_utilities/array_memory_management/array_memory_management.h"
#include "MPI_module/MPI_module.h"
#include "solver/equation_of_state/equation_of_state_2D/equation_of_state_2D.h"
#include "global_initialization.h"
#include "initialize_foreground_2D.h"
#include <math.h>
#include "global_constants.h"
#include "global_parameters.h"
#include <stdlib.h>
#include <mpi.h>

void initialize_foreground_random_oscillations_2D(struct ForegroundVariables2D *fg, struct BackgroundVariables *bg, struct GridInfo2D *grid_info, struct MpiInfo *mpi_info)
{
    /*
    With T=0
    */

    // s1 should be 1% of c_p

    // Random number seed
    srand(1);

    FLOAT_P *p0 = bg->p0;

    // Getting grid info
    int nz_full = grid_info->nz_full;
    int nz_ghost = grid_info->nz_ghost;
    int nz = grid_info->nz;
    int ny = grid_info->ny;
    FLOAT_P dy = grid_info->dy;
    FLOAT_P dz = grid_info->dz;

    FLOAT_P Lz = R_SUN * (R_END - R_START);
    FLOAT_P Ly = R_SUN * Y_SIZE;

    // Calculate pressure scale height
    FLOAT_P *Hp;
    FLOAT_P dp0_dr;
    allocate_1D_array(&Hp, nz_full);

    dp0_dr = (p0[nz_full-1] - p0[nz_full-2]) / (dz);
    Hp[nz_full-1] = - p0[nz_full-1] / dp0_dr;

    dp0_dr = (p0[1] - p0[0]) / (dz);
    Hp[0] = - p0[0] / dp0_dr;

    for (int i = 1; i < nz_full-1; i++)
    {
        dp0_dr = (p0[i+1] - p0[i-1]) / (2.0*dz);
        Hp[i] = - p0[i] / dp0_dr;
    }

    FLOAT_P my_min_Hp = Hp[0];
    for (int i = 1; i < nz_full; i++)
    {
        if (Hp[i] < my_min_Hp)
        {
            my_min_Hp = Hp[i];
        }
    }
    FLOAT_P min_Hp;
    MPI_Allreduce(&my_min_Hp, &min_Hp, 1, MPI_FLOAT_P, MPI_MIN, MPI_COMM_WORLD);

    initialize_foreground_zeros_2D(fg, grid_info);

    // Finding ky and kz
    FLOAT_P *ky, *kz;

    int ky_len = (int)(Ly/min_Hp);
    int kz_len = (int)(Lz/min_Hp);
    printf("ky_len = %d, kz_len = %d\n", ky_len, kz_len);

    allocate_1D_array(&ky, ky_len);
    allocate_1D_array(&kz, kz_len);

    for (int i = 0; i < ky_len; i++)
    {
        ky[i] = i;
    }
    for (int i = 0; i < kz_len; i++)
    {
        kz[i] = i;
    }

    // Random number between -1 and 1
    FLOAT_P A;
    // Random number between 0 and pi
    FLOAT_P phi;

    FLOAT_P sum;
    int rnd_num;

    FLOAT_P z, y;

    FLOAT_P len_sum;

    FLOAT_P r_star = K_B / (MU * M_U);
    FLOAT_P c_p = r_star /(1.0-1.0/GAMMA);

    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        z = bg->r[i];
        for (int j = 0; j < ny; j++)
        {
            y = j * dy;
            sum = 0.0;
            len_sum = 0.0;
            for (int k = 0; k < ky_len; k++)
            {
                for (int l = 0; l < kz_len; l++)
                {
                    A = 2.0 * (FLOAT_P)rand() / (FLOAT_P)RAND_MAX - 1.0;
                    phi = M_PI * (FLOAT_P)rand() / (FLOAT_P)RAND_MAX;
                    
                    rnd_num = rand() % 11;
                    if (rnd_num <=3)
                    {
                        len_sum += 1.0;
                        sum += A * sin(2.0 * M_PI * (ky[k] * y/Ly + kz[l] * z/Lz) + phi);
                    }
                }
            }
            sum = sum / len_sum;
            fg->s1[i][j] = sum;
        }
    }

    // Find minimum and maximum value of s1
    FLOAT_P my_s1_min, my_s1_max;
    my_s1_min = fg->s1[nz_ghost][0];
    my_s1_max = fg->s1[nz_ghost][0];

    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            if (fg->s1[i][j] < my_s1_min)
            {
                my_s1_min = fg->s1[i][j];
            }
            if (fg->s1[i][j] > my_s1_max)
            {
                my_s1_max = fg->s1[i][j];
            }
        }
    }

    FLOAT_P s1_min, s1_max;
    MPI_Allreduce(&my_s1_min, &s1_min, 1, MPI_FLOAT_P, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&my_s1_max, &s1_max, 1, MPI_FLOAT_P, MPI_MAX, MPI_COMM_WORLD);

    // Normalizing s1
    for (int i = nz_ghost; i < nz_full - nz_ghost; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            fg->s1[i][j] = (fg->s1[i][j] - s1_min) / (s1_max - s1_min);
            fg->s1[i][j] = 2.0 * fg->s1[i][j] - 1.0;
            fg->s1[i][j] = fg->s1[i][j] * 0.005 * c_p;

            fg->p1[i][j] = -bg->p0[i] * fg->s1[i][j]/c_p;
        }
    }

    communicate_2D_ghost_above_below(fg->s1, mpi_info, nz, nz_ghost, ny);
    communicate_2D_ghost_above_below(fg->p1, mpi_info, nz, nz_ghost, ny);
    equation_of_state_2D(fg, bg, grid_info, mpi_info);  

    deallocate_1D_array(Hp);
    deallocate_1D_array(ky);
    deallocate_1D_array(kz);
}