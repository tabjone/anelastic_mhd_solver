#ifndef GLOBAL_PARAMETERS_H__
#define GLOBAL_PARAMETERS_H__

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif // M_PI

#define RUN_NAME "spherical" // Name of the run
#define SAVE_DIR "/mn/stornext/d10/data/tabjone/data/"
#define LOAD 1 // 0 for not loading, 1 for loading
#define LOAD_SNAP_NUMBER 70 // Snap number to load

#define BACKGROUND_INTEGRATION_STARTING_POINT 0.985 // Starting point for integration of background
#define BACKGROUND_INTEGRATION_TOP 1.10 // Top point for integration of background
#define BACKGROUND_INTEGRATION_BOTTOM 0.65 // Bottom point for integration of background

#define T 1.0e6 // Simulation time in seconds
#define MAX_DT 1e2 // Maximum time step in seconds
#define SAVE_INTERVAL 1e3 // Save interval in seconds
#define SAVE_ALL 0 // 0 for saving on interval above, 1 for saving all time steps

#define CFL_CUT 0.4 // CFL cut

// Order of the scheme (only upwind 1,2, central 2,4, rk1,2 implemented so far)
#define UPWIND_ORDER 2 // 1 for first order, 2 for second order
#define CENTRAL_ORDER 4 // 2 for second order, 4 for fourth order
#define TIME_ORDER 3 // 1,2,3 for RK1, RK2, RK3

#define COORDINATES 1 // 0 for cartesian, 1 for spherical

// Warning, the units might not be correct in the equations of the system for SI
#define UNITS 0 // 0 for cgs, 1 for SI

// Dimensions
#define DIMENSIONS 2 // 1 for 1D, 2 for 2D, 3 for 3D

// Grid size
#define CZ_START 0.71 // In units of solar radii
// R_START, R_END must be between 0.3 and 1.5
#define R_START 0.68 // In units of solar radii
#define R_END 0.99 // In units of solar radii
#define X_SIZE 0.1 // In units of solar radii
// Y_SIZE is the second coordinate, its theta if polar, so between 0 and 2 pi. Else in solar radii
#define Y_SIZE 1.0 // In units of solar radii
#define NX 50 // Number of grid points in x-direction
#define NY 30000 // Number of grid points in y-direction
#define NZ 250 // Number of grid points in z-direction

// Iterative solver parameters
/* Only use Jacobi for MPI ( Gauss-Seidel is a serial algo )  */
#define ITERATIVE_SOLVER_TYPE 0 // 0 for Jacobi, 1 for Gauss-Seidel, 2 for SOR (not implemented)
#define ITERATIVE_SOLVER_TOLERANCE 1e-6 // Iterative solver tolerance
#define ITERATIVE_SOLVER_MAX_ITERATIONS (int)5e8 // Iterative solver max iterations

// Physical parameters
#define DEL_AD 0.4 // Adiabatic temperature gradient
#define GAMMA (5.0/3.0) // Superadiabatic parameter
#define NABLA_AD 0.4 // Adiabatic temperature gradient

// Background parameters
#define SUPERAD_PARAM 1.0e-4 // superadiabacicity parameter in CZ
// #define p_step 0.001 // Number for determining step-size in space

// Lax parameters
#define LAX_S1 0 // 0 for off, 1 for on
#define LAX_VZ 1 // 0 for off, 1 for on
#define LAX_VY 1 // 0 for off, 1 for on
#define LAX_PARAM_S1 40.0 // Lax parameter for s1
#define LAX_PARAM_VZ 60.0 // Lax parameter for vz
#define LAX_PARAM_VY 60.0 // Lax parameter for vy

// Average removal in velocity
#define REMOVE_AVG_VZ_X 1 // 0 for off, 1 for on
#define AVG_VZ_FACTOR_TOP 180.0 // Factor for removing average in vz
#define AVG_VZ_FACTOR_BOTTOM 12000.0 // Factor for removing average in vz
#define REMOVE_AVG_VY_Z 0 // 0 for off, 1 for on
#define AVG_VY_FACTOR 15000.0 // Factor for removing average in vy


/*
define VISCOSITY_COEFF (1.0e7/5.37) // viscosity coefficient [P] (Poise) (originally 1.0e7/5.37)
define THERMAL_DIFFUSIVITY_COEFF 1.866e6 // thermal diffusivity [...] (orginally 1.866e6)
*/

// Viscosity and thermal diffusivity coefficients
// First coefficient is constant in grid, second is multiplied by gradient
#define FIRST_VISCOSITY_COEFF (1.0e7/5.37) // First viscosity coefficient
#define SECOND_VISCOSITY_COEFF 0.0 // Second viscosity coefficient
#define FIRST_THERMAL_DIFFUSIVITY_COEFF 1.866e6 // First thermal diffusivity coefficient
#define SECOND_THERMAL_DIFFUSIVITY_COEFF 0.0 // Second thermal diffusivity coefficient




#define GRAVITY_ON 1 // 0 for gravity off, 1 for gravity on
#define ADVECTION_ON 1 // 0 for advection off, 1 for advection on
#define GAS_PRESSURE_ON 1 // 0 for gas pressure off, 1 for gas pressure on
#define VISCOSITY_ON 0 // 0 for viscosity off, 1 for viscosity on
#define THERMAL_DIFFUSIVITY_ON 0 // 0 for thermal diffusivity off, 1 for thermal diffusivity on
#define BFIELD_ON 0 // 0 for B-field off, 1 for B-field on
#define CORIOLIS_ON 1 // 0 for Coriolis off, 1 for Coriolis on

#define DEBUG 1
#define SAVE_RHS 0

// Do not change anything below this line
// --------------------------------------

#if UPWIND_ORDER > 2
    #error "Upwind order > 2 not implemented."
#endif // UPWIND_ORDER > 2
#if CENTRAL_ORDER > 4
    #error "Central order > 4 not implemented."
#endif // CENTRAL_ORDER > 4
#if BFIELD_ON != 0
    #error "Only hydrodynamics implemented."
#endif // BFIELD_ON != 0
#if EXTRAPOLATE_GHOST_CELLS != 0
    #error "Only constant extrapolation implemented."
#endif // EXTRAPOLATE_GHOST_CELLS != 0
#endif // GLOBAL_PARAMETERS_H__
