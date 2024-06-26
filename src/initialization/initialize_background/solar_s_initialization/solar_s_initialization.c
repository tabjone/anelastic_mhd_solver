#include <math.h>
#include "global_float_precision.h"
#include "global_parameters.h"
#include "MPI_module/mpi_info_struct.h"
#include "solar_s_initialization.h"
#include "data_structures/background_data/background_variables_struct.h"
#include "array_utilities/array_memory_management/array_memory_management.h"
#include "global_constants.h"
#include "global_initialization.h"
#include "mpi.h"
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif // M_PI

void solar_s_initialization(struct BackgroundVariables *bg, struct MpiInfo *mpi_info, int my_nz_full, int my_nz_ghost, FLOAT_P dz, FLOAT_P my_z0)
{
    int nz_ghost = my_nz_ghost;
    int nz_full = NZ + 2*nz_ghost;

    // Creating radius array for integration that spans entire domain (equal to bg->r if only 1 process)
    FLOAT_P *r_integration;
    allocate_1D_array(&r_integration, nz_full);
    
    for (int i = 0; i < nz_full; i++)
    {
        r_integration[i] = R_START*R_SUN + (i-nz_ghost) * dz;
    }

    // Initializing background radius array
    for (int i = 0; i < my_nz_full; i++)
    {
        bg->r[i] = my_z0 + (i-my_nz_ghost) * dz;
    }
    printf("process %d: my_z0=%2.f", mpi_info->rank, my_z0/R_SUN);

    // Getting initial values from solar_s
    FLOAT_P p0_initial, T0_initial, rho0_initial, m_initial;
    get_initial_values_from_solar_s(&p0_initial, &T0_initial, &rho0_initial, &m_initial, r_integration[nz_full-1]);
    //printf("%Lf %Lf %Lf %Lf\n", p0_initial, T0_initial, rho0_initial, m_initial);


    #if UNITS == 1
        for (int i = 0; i < nz_full; i++)
        {
            r_integration[i] *= 1e-2; // Convert from cm to m
        }
        for (int i = 0; i < my_nz_full; i++)
        {
            bg->r[i] *= 1e-2; // Convert from cm to m
        }
        p0_initial *= 1e3; // Convert from dyn/cm^2 to Pa
        rho0_initial *= 1e-1; // Convert from g/cm^3 to kg/m^3
        m_initial *= 1e-3; // Convert from g to kg
    #endif // UNITS

    // Create arrays for integration variables
    FLOAT_P *p0, *T0, *rho0, *grad_s0, *g, *m;
    allocate_1D_array(&p0, nz_full);
    allocate_1D_array(&T0, nz_full);
    allocate_1D_array(&rho0, nz_full);
    allocate_1D_array(&grad_s0, nz_full);
    allocate_1D_array(&g, nz_full);
    allocate_1D_array(&m, nz_full);

    // Specific heat ratio
    FLOAT_P c_p = K_B / (MU * M_U) /(1.0-1.0/GAMMA);

    FLOAT_P dp_dr, dT_dr, ds_dr, dm_dr, k, nabla_star;

    // Setting initial values for integration arrays
    FLOAT_P r_initial = r_integration[nz_full-1];
    p0[nz_full-1] = p0_initial;
    T0[nz_full-1] = T0_initial;
    rho0[nz_full-1] = get_rho_ideal_gas(p0_initial, T0_initial);
    m[nz_full-1] = m_initial;

    k = get_k_value(r_initial); // Superadiabacity
    nabla_star = NABLA_AD + k; // temperature gradient

    dm_dr = 4 * M_PI * pow(r_initial,2) * rho0_initial;
    dp_dr = - G * m_initial /pow(r_initial,2) * rho0_initial;
    dT_dr = nabla_star * T0_initial/p0_initial * dp_dr;
    ds_dr = k*c_p/ p0_initial * dp_dr;

    // setting initial values for gravity and entropy gradient
    grad_s0[nz_full-1] = ds_dr;
    g[nz_full-1] = G * m_initial / pow(r_initial,2);

    // Doing first integration point by forward Euler
    grad_s0[nz_full-2] = ds_dr;
    m[nz_full-2] = m[nz_full-1] - dm_dr * dz;
    p0[nz_full-2] = p0[nz_full-1] - dp_dr * dz;
    T0[nz_full-2] = T0[nz_full-1] - dT_dr * dz;
    rho0[nz_full-2] = M_U * MU / K_B * p0[nz_full-2]/T0[nz_full-2]; //ideal gas law
    g[nz_full-2] = G * m[nz_full-2] / pow(r_integration[nz_full-2],2);

    // Doing the rest of the integration points by midpoint method
    for (int j = nz_full-2; j > 0; j--)
    {
        k = get_k_value(r_integration[j+1]); // Superadiabacity
        nabla_star = NABLA_AD + k; // temperature gradient

        dm_dr = 4 * M_PI * pow(r_integration[j],2) * rho0[j];
        dp_dr = - G * m[j] /pow(r_integration[j],2) * rho0[j];
        dT_dr = nabla_star * T0[j]/p0[j] * dp_dr;
        ds_dr = k*c_p/ p0[j] * dp_dr;

        grad_s0[j] = ds_dr;
        m[j-1] = m[j+1] - dm_dr * 2*dz;
        p0[j-1] = p0[j+1] - dp_dr * 2*dz;
        T0[j-1] = T0[j+1] - dT_dr * 2*dz;
        rho0[j-1] = get_rho_ideal_gas(p0[j-1], T0[j-1]);
        g[j-1] = G * m[j-1] / pow(r_integration[j-1],2);
    }

    // Last point entropy gradient
    k = get_k_value(r_integration[0]);
    dp_dr = - G * m[0] /pow(r_integration[0],2) * rho0[0];
    grad_s0[0] = k*c_p/ p0[0] * dp_dr;

    #if CONSTANT_BACKGROUND == 1
        // Constant background
        for (int i = 0; i < my_nz_full; i++)
        {
            p0[i] = p0_initial;
            T0[i] = T0_initial;
            rho0[i] = rho0_initial;
            grad_s0[i] = 0.0;
            g[i] = 0.0; // This is the only way to get hydrostatic equilibrium when p0=const
        }
    #endif // CONSTANT_BACKGROUND

    // Sum of all processes nz should be the same as NZ
    // My bg->r[nz_ghost] should be the same as nz_1+nz_2+...+nz_rank
    // Ex: NZ=15 divided by 3 processes, nz_1=5, nz_2=5, nz_3=5
    // Process 0: bg->r[nz_ghost] = r_integration[nz_ghost]
    // Process 1: bg->r[nz_ghost] = r_integration[nz_ghost+nz_1]
    // Process 2: bg->r[nz_ghost] = r_integration[nz_ghost+nz_1+nz_2]

    int sum_nz_below;
    int my_nz = my_nz_full- 2*nz_ghost;

    if (!mpi_info->has_neighbor_below)
    {
        sum_nz_below = 0;
    }

    // Perform the exclusive scan to sum up nz from all processes below the current one
    MPI_Exscan(&my_nz, &sum_nz_below, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    for (int i = 0; i < my_nz_full; i++)
    {
        //bg->r[i] = r_integration[i+sum_nz_below];
        bg->p0[i] = p0[i+sum_nz_below];
        bg->T0[i] = T0[i+sum_nz_below];
        bg->rho0[i] = rho0[i+sum_nz_below];
        bg->grad_s0[i] = grad_s0[i+sum_nz_below];
        bg->g[i] = g[i+sum_nz_below];
    }

    // Checking that bg->r[i] has the same value as r_integration[i+sum_nz_below]
    for (int i = 0; i < my_nz_full; i++)
    {
        if (fabs(bg->r[i] - r_integration[i+sum_nz_below]) > 1.0)
        {
            printf("bg->r[%d] = %f, r_integration[%d] = %f\n", i, bg->r[i], i+sum_nz_below, r_integration[i+sum_nz_below]);
        }
    }

    // Deallocating integration arrays
    deallocate_1D_array(r_integration);
    deallocate_1D_array(p0);
    deallocate_1D_array(T0);
    deallocate_1D_array(rho0);
    deallocate_1D_array(grad_s0);
    deallocate_1D_array(g);
    deallocate_1D_array(m);
}