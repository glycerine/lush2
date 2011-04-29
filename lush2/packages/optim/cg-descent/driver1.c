/* This is the example appearing in the User's Guide with default
   parameter values.  The output on a linux workstation was the following:

   Termination status: 0
   Convergence tolerance for gradient satisfied
   maximum norm for gradient:  9.364889e-09
   function value:            -6.530787e+02

   cg  iterations:                  32
   function evaluations:            55
   gradient evaluations:            45 */

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

    /* allocate space for solution */
    n = 100 ;
    x = (double *) malloc (n*sizeof (double)) ;

    /* set starting guess */
    for (i = 0; i < n; i++) x [i] = 1. ;

    /* run the code */
    cg_descent (x, n, NULL, NULL, 1.e-8, myvalue, mygrad, myvalgrad, NULL) ;

    /* with some loss of efficiency, you could omit the valgrad routine */
    for (i = 0; i < n; i++) x [i] = 1. ; /* starting guess */
    cg_descent (x, n, NULL, NULL, 1.e-8, myvalue, mygrad, NULL, NULL) ;

    free (x) ;
}

double myvalue
(
    double   *x,
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
    double    *g,
    double    *x,
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
