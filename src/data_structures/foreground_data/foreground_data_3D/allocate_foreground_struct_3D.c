#include <stdlib.h>
#include "array_utilities/array_memory_management/array_memory_management.h"
#include "foreground_variables_struct_3D.h"
#include "data_structures/grid_info/grid_info_3D/grid_info_struct_3D.h"

void allocate_foreground_struct_3D(struct ForegroundVariables3D **fg, struct GridInfo3D *grid_info)
{
    int nz_full = grid_info->nz_full;
    int ny = grid_info->ny;
    int nx = grid_info->nx;
    
    *fg = (struct ForegroundVariables3D *)malloc(sizeof(struct ForegroundVariables3D));

    allocate_3D_array(&((*fg)->p1), nz_full, ny, nx);
    allocate_3D_array(&((*fg)->T1), nz_full, ny, nx);
    allocate_3D_array(&((*fg)->rho1), nz_full, ny, nx);
    allocate_3D_array(&((*fg)->s1), nz_full, ny, nx);
    allocate_3D_array(&((*fg)->vx), nz_full, ny, nx);
    allocate_3D_array(&((*fg)->vy), nz_full, ny, nx);
    allocate_3D_array(&((*fg)->vz), nz_full, ny, nx);
}