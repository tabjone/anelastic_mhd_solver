#include "global_float_precision.h"

FLOAT_P central_first_derivative_z_3D(FLOAT_P ***array, int i, int j, int k, FLOAT_P one_over_2dz)
{
    /*
    Calculates the central first derivative of a 2D array at a point (i, j) in the z-direction.

    Parameters
    ----------
    array : FLOAT_P**
        A pointer to the 2D array.
    i : int
        The i index of the point.
    j : int
        The j index of the point.
    dz : FLOAT_P
        The spacing between the points.
    */
    return (array[i+1][j][k] - array[i-1][j][k]) * one_over_2dz;
}