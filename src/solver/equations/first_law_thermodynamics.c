#include "equations.h"

void first_law_thermodynamics(struct ForegroundVariables *fg, struct BackgroundVariables *bg, struct GridInfo *grid_info)
{
    /*
    Calculates the foreground temperature from the first law of thermodynamics.

    Parameters
    ----------
    fg : ForegroundVariables
        A pointer to the ForegroundVariables struct.
    bg : BackgroundVariables
        A pointer to the BackgroundVariables struct.
    grid_info : GridInfo
        A pointer to the GridInfo struct.
    */

    // Calculating spesific gas constant
    FLOAT_P r_star = K_B / (MU * M_U);
    // Spesific heat under constant pressure
    FLOAT_P c_p = r_star /(1.0-1.0/GAMMA);

    #if DIMENSIONS == 1
        // Getting grid info
        int nz_full = grid_info->nz_full;

        for (int i = 0; i < nz_full; i++)
        {
            fg->T1[i] = bg->T0[i]*(fg->s1[i]/c_p + (GAMMA-1)/GAMMA * fg->p1[i]/bg->p0[i]);
        }

    #elif DIMENSIONS == 2
        // Getting grid info
        int ny = grid_info->ny;
        int nz_full = grid_info->nz_full;

        for (int i = 0; i < nz_full; i++)
        {
            for (int j = 0; j < ny; j++)
            {
                fg->T1[i][j] = bg->T0[i]*(fg->s1[i][j]/c_p + (GAMMA-1)/GAMMA * fg->p1[i][j]/bg->p0[i]);
            }
        }

    #elif DIMENSIONS == 3
        // Getting grid info
        int nx = grid_info->nx;
        int ny = grid_info->ny;
        int nz_full = grid_info->nz_full;

        for (int i = 0; i < nz_full; i++)
        {
            for (int j = 0; j < ny; j++)
            {
                for (int k = 0; k < nx; k++)
                {
                    fg->T1[i][j][k] = bg->T0[i]*(fg->s1[i][j][k]/c_p + (GAMMA-1)/GAMMA * fg->p1[i][j][k]/bg->p0[i]);
                }
            }
        }
    #endif // DIMENSIONS
}