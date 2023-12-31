#include <stdlib.h>
#include "global_float_precision.h"

void deallocate_3D_array(FLOAT_P ***array_ptr) 
{
    /*
    Deallocates memory for a 3D contiguous array.

    Parameters
    ----------
    array_ptr : FLOAT_P***
        Pointer to the 3D array to deallocate.
    */

    free(array_ptr[0][0]);  // Free the actual 3D array
    free(array_ptr[0]);     // Free the array of pointers to each row of each 2D slice
    free(array_ptr);        // Free the primary array of pointers to each 2D slice
    array_ptr = NULL;      // Set the pointer to NULL to avoid dangling pointer
}