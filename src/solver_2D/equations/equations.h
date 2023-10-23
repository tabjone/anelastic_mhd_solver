#ifndef EQUATIONS_H__
#define EQUATIONS_H__

#include "shared_files.h"
#include "global_parameters.h"

void equation_of_state(struct ForegroundVariables2D *fg, struct BackgroundVariables *bg, struct GridInfo2D *grid_info);
void first_law_thermodynamics(struct ForegroundVariables2D *fg, struct BackgroundVariables *bg, struct GridInfo2D *grid_info);

#endif // EQUATIONS_H__