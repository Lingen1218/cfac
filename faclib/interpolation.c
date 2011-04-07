#include <gsl/gsl_spline.h>
#include <gsl/gsl_linalg.h>

#include "consts.h"
#include "global.h"
#include "interpolation.h"
#include "cf77.h"

/* closed Newton-Cotes formulae coeff. */
static double _CNC[5][5] = {
  {0, 0, 0, 0, 0},
  {0.5, 0.5, 0, 0, 0},
  {1.0/3.0, 4.0/3.0, 1.0/3.0, 0, 0},
  {3.0/8, 9.0/8, 9.0/8, 3.0/8, 0},
  {14.0/45, 64.0/45, 24.0/45, 64.0/45, 14.0/45,}
};

static struct {
  double *x;
  double *logx;
  double *y;
  double *sigma;
  void *extra;
  void (*function)(int, double *, int , double *, double *, 
		   double *, double *, int, void *);
} minpack_params;


void SVDFit(int np, double *coeff, double *chisq, double tol,
	    int nd, double *x, double *logx, double *y, double *sig,
	    void Basis(int, double *, double, double)) {
  int i, j;
  double *afunc;
  double thresh, wmax;
  int infor;
  gsl_matrix *Am, *Vm;
  gsl_vector *bv, *Sv, *wv;
  gsl_vector_view xv;
 
  Am = gsl_matrix_alloc(nd, np);
  Vm = gsl_matrix_alloc(np, np);
  bv = gsl_vector_alloc(nd);
  Sv = gsl_vector_alloc(np);
  
  wv = gsl_vector_alloc(np);

  xv = gsl_vector_view_array(coeff, np);

  afunc = malloc(sizeof(double)*np);

  for (i = 0; i < nd; i++) {
    double tmp;
    
    if (logx) {
      Basis(np, afunc, x[i], logx[i]);
    } else {
      Basis(np, afunc, x[i], 0.0);
    }
    
    if (sig) {
      tmp = 1.0/sig[i];
    } else {
      tmp = 1.0;
    }
    
    for (j = 0; j < np; j++) {
      gsl_matrix_set(Am, i, j, afunc[j]*tmp);
    }
    gsl_vector_set(bv, i, y[i]*tmp);
  }
  
  free(afunc);

  infor = gsl_linalg_SV_decomp(Am, Vm, Sv, wv);
  
  gsl_vector_free(wv);
    
  if (infor != 0) {
    fprintf(stderr, "gsl_linalg_SV_decomp() failed with %d\n", infor);
    abort();
  }
    
  wmax = gsl_vector_get(Sv, 0);
  thresh = tol*wmax;
  for (j = 0; j < np; j++) {
    if (gsl_vector_get(Sv, j) < thresh) gsl_vector_set(Sv, j, 0.0);
  }

  gsl_linalg_SV_solve(Am, Vm, Sv, bv, &xv.vector);
  
  gsl_vector_free(bv);
  gsl_vector_free(Sv);
  gsl_matrix_free(Am);
  gsl_matrix_free(Vm);
}

static void MinFunc(int *m, int *n, double *x, double *fvec, 
		    double *fjac, int *ldfjac, int *iflag) {
  int ndy, i, j, k;

  if (*iflag == 1) {
    ndy = 0;
    minpack_params.function(*n, x, *m, minpack_params.x,
			    minpack_params.logx, fvec, 
			    NULL, ndy, minpack_params.extra);
    for (i = 0; i < *m; i++) {
      fvec[i] = fvec[i] - minpack_params.y[i];
      if (minpack_params.sigma) fvec[i] = fvec[i]/minpack_params.sigma[i];
    }
  } else if (*iflag == 2){
    ndy = *ldfjac;
    minpack_params.function(*n, x, *m, minpack_params.x,
			    minpack_params.logx, NULL, 
			    fjac, ndy, minpack_params.extra);
    if (minpack_params.sigma) {
      for (i = 0; i < *m; i++) {
	k = i;
	for (j = 0; j < *n; j++) {
	  fjac[k] = fjac[k]/minpack_params.sigma[i];
	  k += ndy;
	}
      }
    }
  }
}
/* provide fortran access with cfortran.h */
FCALLSCSUB7(MinFunc, MINFUNC, minfunc, PINT, PINT, 
	    DOUBLEV, DOUBLEV, DOUBLEV, PINT, PINT)
    
int NLSQFit(int np, double *p, double tol, int *ipvt,
	    double *fvec, double *fjac, int ldfjac, double *wa, int lwa,
	    int n, double *x, double *logx, double *y, double *sig,
	    void Func(int, double *, int , double *, double *, 
		      double *, double *, int, void *), 
	    void *extra) {
  int info, nprint, nfev, njev;
  int maxfev, mode;
  double factor, *diag, *qtf, *wa1, *wa2, *wa3, *wa4;
  double zero = 0.0;
  double ftol;

  minpack_params.x = x;
  minpack_params.logx = logx;
  minpack_params.y = y;
  minpack_params.sigma = sig;
  minpack_params.extra = extra;
  minpack_params.function = Func;
  
  diag = wa;
  qtf = wa+np;
  wa1 = qtf+np;
  wa2 = wa1+np;
  wa3 = wa2+np;
  wa4 = wa3+np;
  mode = 1;
  /*
  MinFunc(&n, &np, p, fvec, fjac, &ldfjac, &mode);
  CHKDER(n, np, p, fvec, fjac, ldfjac, wa3, wa4, mode, diag);
  MinFunc(&n, &np, wa3, wa4, fjac, &ldfjac, &mode);
  mode = 2;
  MinFunc(&n, &np, p, fvec, fjac, &ldfjac, &mode);
  CHKDER(n, np, p, fvec, fjac, ldfjac, wa3, wa4, mode, diag);
  for (nfev = 0; nfev < n; nfev++) {
    printf("%d %10.3E\n", nfev, diag[nfev]);
  }
  exit(1);
  */
  maxfev = 5000*np;
  mode = 1;
  nprint = 0;
  factor = 100.0;
  ftol = tol*tol*n;
  LMDER(C_FUNCTION(MINFUNC, minfunc), n, np, p, fvec, fjac, ldfjac, 
	ftol, tol, zero, maxfev, diag, mode, factor,
	nprint, &info, &nfev, &njev, ipvt, qtf, wa1, wa2, wa3, wa4);

  return info;
}


double Simpson(double *x, int i0, int i1) {
  int i, k;
  double a, b;

  b = x[i0];
  a = 0.0;
  for (i = i0+1; i < i1; i += 2) {
    a += x[i];
  }
  b += 4.0*a;
  a = 0.0;
  k = i1-1;
  for (i = i0+2; i < k; i += 2) {
    a += x[i];
  }
  b += 2.0*a;
  if (i == i1) {
    b += x[i1];
    b /= 3.0;
  } else {
    b += x[k];
    b /= 3.0;
    b += 0.5*(x[k] + x[i1]);
  }

  return b;
}

/*
 * Integration by Newton-Cotes formula
 * input: x[]
 * limits (indices): 0 .. ilast
 * output: r[]
 * last_only: only the finite integral is needed
 * id - direction (<0 => inward)
 */
int NewtonCotes(double r[], const double x[], int ilast,
                int last_only, int id) {
  int i, k;
  double a;

  if (id >= 0) {
    if (last_only) {
      r[ilast] = x[0];
      a = 0.0;
      for (i = 1; i < ilast; i += 2) {
	a += x[i];
      }
      r[ilast] += 4.0*a;
      a = 0.0;
      k = ilast-1;
      for (i = 2; i < k; i += 2) {
	a += x[i];
      }
      r[ilast] += 2.0*a;
      if (i == ilast) {
	r[ilast] += x[ilast];
	r[ilast] /= 3.0;
      } else {
	r[ilast] += x[k];
	r[ilast] /= 3.0;
	r[ilast] += 0.5*(x[k] + x[ilast]);
      }
      r[ilast] += r[0];
    } else {
      for (i = 2; i <= ilast; i += 2) {
	r[i] = r[i-2] + _CNC[2][0]*(x[i-2]+x[i]) + _CNC[2][1]*x[i-1];
	r[i-1] = r[i-2] + 0.5*(x[i-2] + x[i-1]);
      }
      if (i == ilast+1) {
	k = ilast-1;
	r[ilast] = r[k] + 0.5*(x[k]+x[ilast]);
      }
    }
  } else {
    if (last_only) {
      r[ilast] = x[ilast];
      a = 0.0;
      for (i = ilast-1; i > 0; i -= 2) {
	a += x[i];
      }
      r[ilast] += 4.0*a;
      a = 0.0;
      k = 1;
      for (i = ilast-2; i > k; i -= 2) {
	a += x[i];
      }
      r[ilast] += 2.0*a;
      if (i == 0) {
	r[ilast] += x[0];
	r[ilast] /= 3.0;
      } else {
	r[ilast] += x[k];
	r[ilast] /= 3.0;
	r[ilast] += 0.5*(x[k] + x[0]);
      }
      r[ilast] += r[0];
    } else {
      for (i = ilast-2; i >= 0; i -= 2) {
	r[i] = r[i+2] + _CNC[2][0]*(x[i+2]+x[i]) + _CNC[2][1]*x[i+1];
	r[i+1] = r[i+2] + 0.5*(x[i+2] + x[i+1]);
      }
      if (i == -1) {
	k = 1;
	r[0] = r[k] + 0.5*(x[k]+x[0]);
      }
    }
  }

  return 0;
}

/* Imitate F77 UVIP3P by Hiroshi Akima (NP=3) */
void uvip3p(int nd, const double *xd, const double *yd,
		 int ni, const double *xi, double *yi)
{
    int i;
    const gsl_interp_type *type;
    gsl_interp_accel *acc;
    gsl_spline *spline;
    
    switch (nd) {
    case 2:
        type = gsl_interp_linear;
        break;
    case 3:
    case 4:
        type = gsl_interp_cspline;
        break;
    default:
        type = gsl_interp_akima;
        break;
    }
    
    acc = gsl_interp_accel_alloc();
    spline = gsl_spline_alloc(type, nd);

    gsl_spline_init(spline, xd, yd, nd);

    for (i = 0; i < ni; i++) {
        yi[i] = gsl_spline_eval(spline, xi[i], acc);
    }
    
    gsl_spline_free(spline);
    gsl_interp_accel_free(acc);

    return;
}

/* private GSL structures... */
typedef struct
{
  double * c;
  double * g;
  double * diag;
  double * offdiag;
} cspline_state_t;

typedef struct
{
  double * b;
  double * c;
  double * d;
  double * _m;
} akima_state_t;

static inline void
coeff_calc (const double c_array[], double dy, double dx, size_t index,  
            double * b, double * c, double * d)
{
  const double c_i = c_array[index];
  const double c_ip1 = c_array[index + 1];
  *b = (dy / dx) - dx * (c_ip1 + 2.0 * c_i) / 3.0;
  *c = c_i;
  *d = (c_ip1 - c_i) / (3.0 * dx);
}

/* Imitate F77 UVIP3C (NP=3) */
void uvip3c(int nd, const double xd[], const double yd[],
		 double c1[], double c2[], double c3[])
{
    int i;
    const gsl_interp_type *type;
    gsl_interp_accel *acc;
    gsl_spline *spline;
    gsl_interp *interp;
    
    
    if (nd > 4) {
        type = gsl_interp_akima;
    } else
    if (nd > 2) {
        type = gsl_interp_cspline;
    } else {
        fprintf(stderr, "uvip3c() called with nd = %d\n", nd);
        abort();
    }
    
    acc = gsl_interp_accel_alloc();
    spline = gsl_spline_alloc(type, nd);

    gsl_spline_init(spline, xd, yd, nd);
    
    interp = spline->interp;
    
    if (nd < 5) {
        const cspline_state_t* cs = (cspline_state_t *) interp->state;
        for (i = 0; i < nd - 1; i++) {
            double dx = xd[i + 1] - xd[i];
            double dy = yd[i + 1] - yd[i];
            
            coeff_calc(cs->c, dy, dx, i, &c1[i], &c2[i], &c3[i]);
        }
        
    } else {
        const akima_state_t* as = (akima_state_t *) interp->state;
        memcpy(c1, as->b, sizeof(double)*(nd - 1));
        memcpy(c2, as->c, sizeof(double)*(nd - 1));
        memcpy(c3, as->d, sizeof(double)*(nd - 1));
    }

    gsl_spline_free(spline);
    gsl_interp_accel_free(acc);

    return;
}
