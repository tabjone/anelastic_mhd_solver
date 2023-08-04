#include "solar_s_initialization.h"

void interpolate_solar_s(double *r_over_R_solar_s, double *c_s_solar_s, double *rho_solar_s, double *p_solar_s, double *Gamma_1_solar_s, double *T_solar_s, double *r_over_R_i, double *c_s_i, double *rho_i, double *p_i, double *Gamma_1_i, double *T_i, int solar_s_size)
{
    /*
    Interpolates the solar_s data to the grid at a point i.

    Parameters
    ----------
    r_over_R_solar_s : double*
        The r/R values of the solar_s data.
    c_s_solar_s : double*
        The sound speed values of the solar_s data in cm/s.
    rho_solar_s : double*
        The density values of the solar_s data in g/cm^3.
    p_solar_s : double*
        The pressure values of the solar_s data in dyn/cm^2.
    Gamma_1_solar_s : double*
        The first adiabatic index values of the solar_s data.
    T_solar_s : double*
        The temperature values of the solar_s data in K.
    r_over_R_i : double*
        Address of the r/R values of the i'th point in the grid.
    c_s_i : double*
        Address of the sound speed values of the i'th point in the grid in cm/s.
    rho_is : double*
        Address of the density values of the i'th point in the grid in g/cm^3.
    p_i : double*
        Address of the pressure values of the i'th point in the grid in dyn/cm^2.
    Gamma_1_i : double*
        Address of the first adiabatic index values of the i'th point in the grid.
    T_i : double*
        Address of the temperature values of the i'th point in the grid in K.
    solar_s_size : int
        The size of the solar_s data arrays.
    */

    // Handle edge cases, remember that r_over_R_solar_s is sorted in decending order
    if (*r_over_R_i >= r_over_R_solar_s[0])
    {
        *c_s_i = c_s_solar_s[0];
        *rho_i = rho_solar_s[0];
        *p_i = p_solar_s[0];
        *Gamma_1_i = Gamma_1_solar_s[0];
        *T_i = T_solar_s[0];
        return;
    }
    else if (*r_over_R_i <= r_over_R_solar_s[solar_s_size - 1])
    {
        *c_s_i = c_s_solar_s[solar_s_size - 1];
        *rho_i = rho_solar_s[solar_s_size - 1];
        *p_i = p_solar_s[solar_s_size - 1];
        *Gamma_1_i = Gamma_1_solar_s[solar_s_size - 1];
        *T_i = T_solar_s[solar_s_size - 1];
        return;
    }

    // Search for the target interval
    int i = 0;
    while (r_over_R_solar_s[i] > *r_over_R_i) 
    {
        i++;
    } 
    /*
    Adding 1 to i here will make sure we take the values around the target point, but might create trouble if the target point is the last point in the solar_s data. So we should have a function that checks if the i+1 value is out of bounds and if so, just use the i value.
    */
    i = i + 1;

    // Perform the interpolation
    double x0 = r_over_R_solar_s[i-1];
    double x1 = r_over_R_solar_s[i];

    *c_s_i = interpolate_1D(x0, x1, c_s_solar_s[i-1], c_s_solar_s[i], *r_over_R_i);
    *rho_i = interpolate_1D(x0, x1, rho_solar_s[i-1], rho_solar_s[i], *r_over_R_i);
    *p_i = interpolate_1D(x0, x1, p_solar_s[i-1], p_solar_s[i], *r_over_R_i);
    *Gamma_1_i = interpolate_1D(x0, x1, Gamma_1_solar_s[i-1], Gamma_1_solar_s[i], *r_over_R_i);
    *T_i = interpolate_1D(x0, x1, T_solar_s[i-1], T_solar_s[i], *r_over_R_i);
}