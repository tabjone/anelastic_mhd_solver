#include "solar_s_initialization.h"
#include <math.h>

void integrate_one_step(struct IntegrationVariables *bg, int i, bool updown)
{
    // false for upward integration, true for downward integration
    FLOAT_P k = get_k_value(bg->r[i]);
    FLOAT_P nabla_star = NABLA_AD + k;

    FLOAT_P dm_dr = 4 * M_PI * pow(bg->r[i],2) * bg->rho0[i];
    FLOAT_P dp_dr = - G * bg->m[i] /pow(bg->r[i],2) * bg->rho0[i];
    FLOAT_P dT_dr = nabla_star * bg->T0[i]/bg->p0[i] * dp_dr;
    FLOAT_P r_star = K_B / (MU * M_U);
    FLOAT_P c_p = r_star /(1.0-1.0/GAMMA);
    FLOAT_P ds_dr = - c_p * (nabla_star - NABLA_AD) / bg->p0[i] * dp_dr;

    if ((i+1) % bg->N == 0 && i != 0)
    {
        bg->r = realloc(bg->r, sizeof(FLOAT_P)*(bg->N*bg->N_increment));
        bg->p0 = realloc(bg->p0, sizeof(FLOAT_P)*(bg->N*bg->N_increment));
        bg->T0 = realloc(bg->T0, sizeof(FLOAT_P)*(bg->N*bg->N_increment));
        bg->rho0 = realloc(bg->rho0, sizeof(FLOAT_P)*(bg->N*bg->N_increment));
        bg->s0 = realloc(bg->s0, sizeof(FLOAT_P)*(bg->N*bg->N_increment));
        bg->grad_s0 = realloc(bg->grad_s0, sizeof(FLOAT_P)*(bg->N*bg->N_increment));
        bg->m = realloc(bg->m, sizeof(FLOAT_P)*(bg->N*bg->N_increment));

        bg->N_increment = bg->N_increment + 1;
    }

    FLOAT_P dr1 = fabs(p_step * bg->m[i]/dm_dr);
    FLOAT_P dr2 = fabs(p_step * bg->p0[i]/dp_dr);
    FLOAT_P dr3 = fabs(p_step * bg->T0[i]/dT_dr);
    FLOAT_P dr4 = fabs(p_step * bg->s0[i]/ds_dr);

    FLOAT_P dr;
    if (dr1 < dr2 && dr1 < dr3 && dr1 < dr4) {
        dr = dr1;
    } else if (dr2 < dr1 && dr2 < dr3 && dr2 < dr4) {
        dr = dr2;
    } else if (dr3 < dr1 && dr3 < dr2 && dr3 < dr4) {
        dr = dr3;
    } else {
        dr = dr4;
    }
    if (updown)
    {
        dr = -dr;
    }
    
    bg->grad_s0[i] = ds_dr;
    
    bg->r[i+1] = bg->r[i] + dr;
    bg->m[i+1] = bg->m[i] + dm_dr * dr;
    bg->p0[i+1] = bg->p0[i] + dp_dr * dr;
    bg->T0[i+1] = bg->T0[i] + dT_dr * dr;
    bg->s0[i+1] = bg->s0[i] + ds_dr * dr;
    bg->rho0[i+1] = M_U * MU / K_B * bg->p0[i+1]/bg->T0[i+1]; //ideal gas law
}
