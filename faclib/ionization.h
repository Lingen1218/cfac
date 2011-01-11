#ifndef _IONIZATION_H_
#define _IONIZATION_H_

int FreeIonizationQk(void);
int InitIonization(void);
int ReinitIonization(int m);
int SetIEGrid(int n, double emin, double emax);
int SetIEGridDetail(int n, double *x);
void SetCIBorn(int x);
void SetCILQR(int m);
void SetCILMax(int m);
void SetCILMaxEject(int m);
void SetCILCB(int m);
void SetCITol(double t);
int SetCIPWOptions(int qr, int max, int max_eject, int kl_cb, double tol);
int SetCIPWGrid(int ns, int *n, int *step);
int SetCIFormat(int m);
int SetCIEGrid(int n, double emin, double emax, double eth);
int SetCIEGridDetail(int n, double *x);
int SetCIFormat(int m);
int SetCIEGridLimits(double min, double max, int type);
int SetUsrCIEGridType(int type);
int SetUsrCIEGrid(int n, double emin, double emax, double eth);
int SetUsrCIEGridDetail(int n, double *x);
int SetCIQkMode(int m, double tol);
int SetCIMaxK(int k);
int CIRadialQk(double *qk, double e1, double e2, int kb, int kbp, int k);
int CIRadialQkIntegrated(double *qku, double te, int kb, int kbp);
void CIRadialQkBasis(int npar, double *yb, double x, double logx);
void CIRadialQkFromFit(int np, double *p, int n, 
		       double *x, double *logx, double *y);
int CIRadialQkBED(double *dp, double *bethe, double *b0, int kl,
		  double *logxe, double *q, double *p, double te);
double *CIRadialQkIntegratedTable(int kb, int kbp);
int IonizeStrength(double *qku, double *p, double *e, int b, int f);
int SaveIonization(int nb, int *b, int nf, int *f, char *fn);
double CIRadialQkMSub(int J0, int M0, int J1, int M1, int k0, int k1, 
		      double e1, double e2, double e0);
double CIRadialQkIntegratedMSub(int j1, int m1, int j2, int m2,
				int k0, int k1, double te, double e12);
int IonizeStrengthMSub(double *qku, double *e, int b, int f);
int SaveIonizationMSub(int nb, int *b, int nf, int *f, char *fn);
double BEScale(int k, double e);

#endif
