#include "structs.h"
#include <stdlib.h>

void deallocate_background_struct(struct BackgroundVariables *bg)
{
    deallocate_1D_array(bg->r);
    deallocate_1D_array(bg->p0);
    deallocate_1D_array(bg->T0);
    deallocate_1D_array(bg->rho0);
    deallocate_1D_array(bg->one_over_rho0);
    deallocate_1D_array(bg->grad_s0);
    deallocate_1D_array(bg->g);
    deallocate_1D_array(bg->grad_g);
    deallocate_1D_array(bg->grad_rho0);
    deallocate_1D_array(bg->eta_over_four_pi_rho0_T0);

    free(bg);
}