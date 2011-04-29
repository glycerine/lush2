/* In the line search for first iteration, there is very little information
   available for choosing a suitable stepsize. By default, the code employs
   very low order approximations to the function to estimate a suitable
   stepsize. In some cases, this initial stepsize can be problematic.
   For example, if the cost function contains a log function, the initial
   step might cause the code to try to compute the log of a negative number.
   If the cost function contains an exponential, then the initial stepsize
   might lead to an overflow. In either case, NaNs are potentially generated.
   If the default stepsize is unsuitable, you can input the starting
   stepsize using the parameter ``step''.
   In the following example, the initial stepsize is set to 1.
   The output is below:

   Termination status: 0
   Convergence tolerance for gradient satisfied
   maximum norm for gradient:  7.686512e-09
   function value:            -6.530787e+02

   cg  iterations:                  31
   function evaluations:            52
   gradient evaluations:            43 */

#include <math.h>
#include "cg_user.h"

double myvalue
(
    double   *x,
    INT       n
) ;

void mygrad
(
    double    *g,
    double    *x,
    INT        n
) ;
double myvalgrad
(
    double    *g,
    double    *x,
    INT        n
) ;

int main (void)
{
    double *x ;
    INT i, n ;
    cg_stats Stats ;
    cg_parameter Parm ;

    /* allocate space for solution */
    n = 100 ;
    x = (double *) malloc (n*sizeof (double)) ;

    /* starting guess */
    for (i = 0; i < n; i++) x [i] = 1. ;

    /* initialize default parameter values */
    cg_default (&Parm) ;

    /* set parameter step = 1. */
    Parm.step = 1. ;

    /* solve the problem */
    cg_descent(x, n, &Stats, &Parm, 1.e-8, myvalue, mygrad, myvalgrad, NULL) ;

    free (x) ; /* free work space */
}

double myvalue
(
    double   *x ,
    INT       n
)
{
    double f, t ;
    INT i ;
    f = 0. ;
    for (i = 0; i < n; i++)
    {
        t = i+1 ;
        t = sqrt (t) ;
        f += exp (x [i]) - t*x [i] ;
    }
    return (f) ;
}

void mygrad
(
    double    *g ,
    double    *x ,
    INT        n
)
{
    double t ;
    INT i ;
    for (i = 0; i < n; i++)
    {
        t = i + 1 ;
        t = sqrt (t) ;
        g [i] = exp (x [i]) -  t ;
    }
    return ;
}

double myvalgrad
(
    double    *g,
    double    *x,
    INT        n
)
{
    double ex, f, t ;
    INT i ;
    f = (double) 0 ;
    for (i = 0; i < n; i++)
    {
        t = i + 1 ;
        t = sqrt (t) ;
        ex = exp (x [i]) ;
        f += ex - t*x [i] ;
        g [i] = ex -  t ;
    }
    return (f) ;
}
