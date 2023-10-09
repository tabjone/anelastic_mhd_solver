#include "extrapolation.h"

#if DIMENSIONS == 1
void extrapolate_1D_array_constant_down(FLOAT_P *array, struct GridInfo *grid_info)
{
    /*
    Extrapolates the ghost cells of a 1D array using constant extrapolation in the downward direction.

    Parameters
    ----------
    array : FLOAT_P*
        A pointer to the array to be extrapolated.
    grid_info : GridInfo
        A pointer to the GridInfo struct.
    */

    // Getting grid info
    int nz_ghost = grid_info->nz_ghost;

    // Extrapolating
    for (int i = 0; i < nz_ghost; i++)
    {
        array[i] = array[nz_ghost];
    }
}
#endif // DIMENSIONS