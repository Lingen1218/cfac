#include <gsl/gsl_spline.h>

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


void PolyBasis(int n, double *y, double x, double logx) {
  int i;
  double t;
  
  t = 1.0;
  for (i = 0; i < n; i++) {
    y[i] = t;
    t *= x;
  }
}

void PolyFit(int n, double *c, int nd, double *x, double *y) {
  SVDFit(n, c, NULL, EPS6, nd, x, NULL, y, NULL, PolyBasis);
}

void SVDFit(int np, double *coeff, double *chisq, double tol,
	    int nd, double *x, double *logx, double *y, double *sig,
	    void Basis(int, double *, double, double)) {
  double *u, *v, *w, *b, *afunc;
  int i, j, k;
  double tmp, thresh, wmax, sum;
  double *dwork;
  int *iwork, lwork, infor;
  char jobz[] = "O";
 
  k = np*np;
  lwork = 5*k + 4*np;
  if (lwork < nd) lwork = nd;
  lwork += 3*k;
  lwork *= 2;
  i = (np+nd)*(np+1)+np;
  w = (double *) malloc(sizeof(double)*(i+lwork));
  iwork = (int *) malloc(sizeof(int)*8*np);

  v = w + np;
  u = v + np*np;
  b = u + np*nd;
  afunc = b + nd;
  dwork = afunc + np;
  for (i = 0; i < nd; i++) {
    if (logx) {
      Basis(np, afunc, x[i], logx[i]);
    } else {
      Basis(np, afunc, x[i], 0.0);
    }
    k = i;
    if (sig) {
      tmp = 1.0/sig[i];
      for (j = 0; j < np; j++) {
	u[k] = afunc[j]*tmp;
	k += nd;
      }
      b[i] = y[i]*tmp;
    } else {
      for (j = 0; j < np; j++) {
	u[k] = afunc[j];
	k += nd;
      }
      b[i] = y[i];
    }
  }

  DGESDD(jobz, nd, np, u, nd, w, u, nd, v, np,
	  dwork, lwork, iwork, &infor);
    
  wmax = w[0];
  thresh = tol*wmax;
  for (j = 0; j < np; j++) {
    if (w[j] < thresh) w[j] = 0.0;
  }
  
  k = 0;
  for (j = 0; j < np; j++) {
    tmp = 0.0;
    if (w[j] != 0.0) {
      for (i = 0; i < nd; i++) {
	tmp += u[k++]*b[i];
      }
      tmp /= w[j];
    }
    afunc[j] = tmp;
  }
  
  k = 0;
  for (j = 0; j < np; j++) {
    tmp = 0.0;
    for (i = 0; i < np; i++) {
      tmp += v[k++]*afunc[i];
    }
    coeff[j] = tmp;
  }
  
  if (chisq) {
    *chisq = 0.0;
    for (i = 0; i < nd; i++) {
      if (logx) {
	Basis(np, afunc, x[i], logx[i]);
      } else {
	Basis(np, afunc, x[i], 0);
      }
      sum = 0.0;
      for (j = 0; j < np; j++) {
	sum += coeff[j]*afunc[j];
      }
      tmp = y[i] - sum;
      if (sig) tmp /= sig[i];
      *chisq += tmp*tmp;
    }
  }  
  
  free(w);
  free(iwork);
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

/* integration by newton-cotes formula */
int NewtonCotes(double *r, double *x, int i0, int i1, int m, int id) {
  int i, k;
  double a;

  if (id >= 0) {
    if (m >= 0) {
      r[i1] = x[i0];
      a = 0.0;
      for (i = i0+1; i < i1; i += 2) {
	a += x[i];
      }
      r[i1] += 4.0*a;
      a = 0.0;
      k = i1-1;
      for (i = i0+2; i < k; i += 2) {
	a += x[i];
      }
      r[i1] += 2.0*a;
      if (i == i1) {
	r[i1] += x[i1];
	r[i1] /= 3.0;
      } else {
	r[i1] += x[k];
	r[i1] /= 3.0;
	r[i1] += 0.5*(x[k] + x[i1]);
      }
      r[i1] += r[i0];
    } else {
      for (i = i0+2; i <= i1; i += 2) {
	r[i] = r[i-2] + _CNC[2][0]*(x[i-2]+x[i]) + _CNC[2][1]*x[i-1];
	r[i-1] = r[i-2] + 0.5*(x[i-2] + x[i-1]);
      }
      if (i == i1+1) {
	k = i1-1;
	r[i1] = r[k] + 0.5*(x[k]+x[i1]);
      }
    }
  } else {
    if (m >= 0) {
      r[i1] = x[i1];
      a = 0.0;
      for (i = i1-1; i > i0; i -= 2) {
	a += x[i];
      }
      r[i1] += 4.0*a;
      a = 0.0;
      k = i0+1;
      for (i = i1-2; i > k; i -= 2) {
	a += x[i];
      }
      r[i1] += 2.0*a;
      if (i == i0) {
	r[i1] += x[i0];
	r[i1] /= 3.0;
      } else {
	r[i1] += x[k];
	r[i1] /= 3.0;
	r[i1] += 0.5*(x[k] + x[i0]);
      }
      r[i1] += r[i0];
    } else {
      for (i = i1-2; i >= i0; i -= 2) {
	r[i] = r[i+2] + _CNC[2][0]*(x[i+2]+x[i]) + _CNC[2][1]*x[i+1];
	r[i+1] = r[i+2] + 0.5*(x[i+2] + x[i+1]);
      }
      if (i == i0-1) {
	k = i0+1;
	r[i0] = r[k] + 0.5*(x[k]+x[i0]);
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
