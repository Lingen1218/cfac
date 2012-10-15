#ifndef _RADIAL_H_
#define _RADIAL_H_

#include "config.h"
#include "orbital.h"

typedef enum {
    INT_P1P2pQ1Q2 = 1,    /* P1*P2 + Q1*Q2 */
    INT_P1P2      = 2,    /* P1*P2         */
    INT_Q1Q2      = 3,    /* Q1*Q2         */ 
    INT_P1Q2pQ1P2 = 4,    /* P1*Q2 + Q1*P2 */
    INT_P1Q2mQ1P2 = 5,    /* P1*Q2 - Q1*P2 */
    INT_P1Q2      = 6,    /* P1*Q2         */
    INT_Q1P2      = 7     /* P1*Q2         */
} RadIntType;

typedef struct _SLATER_YK_ {
  short npts;
  float *yk;
  float coeff[2];
} SLATER_YK;

double *WLarge(ORBITAL *orb);
double *WSmall(ORBITAL *orb);
int GetBoundary(double *rb, double *b, int *nmax, double *dr);
int SetBoundary(int nmax, double p, double bqp);
int RadialOverlaps(char *fn, int kappa);
void SetSlaterCut(int k0, int k1);
void SetPotentialMode(int m, double h);
void SetSE(int n);
void SetVP(int n);
void SetBreit(int n);
void SetMS(int nms, int sms);
int SetAWGrid(int n, double min, double max);
int GetAWGrid(double **a);
int SetRadialGrid(int maxrp, double ratio, double asymp, double rmin);
double SetPotential(AVERAGE_CONFIG *acfg, int iter, double *vbuf);
POTENTIAL *RadialPotential(void);
int GetPotential(char *s);
double GetResidualZ(void);
double GetRMax(void);

/* solve the dirac equation for the given orbital */
int SolveDirac(ORBITAL *orb);
int WaveFuncTable(char *s, int n, int kappa, double e);

/* get the index of the given orbital in the table */
int OrbitalIndex(int n, int kappa, double energy);
int OrbitalExists(int n, int kappa, double energy);
int AddOrbital(ORBITAL *orb);
ORBITAL *GetOrbital(int k);
ORBITAL *GetOrbitalSolved(int k);
ORBITAL *GetNewOrbital(void);
int GetNumBounds(void);
int GetNumOrbitals(void);
int GetNumContinua(void);

double CoulombEnergyShell(CONFIG *cfg, int i);
void ShiftOrbitalEnergy(CONFIG *cfg);
double GetPhaseShift(int k);

/* radial optimization */
int SetAverageConfig(int nshells, int *n, int *kappa, double *nq);
void SetOptimizeMaxIter(int m);
void SetOptimizeStabilizer(double m);
void SetOptimizeTolerance(double c);
void SetOptimizePrint(int m);
void SetOptimizeControl(double tolerence, double stablizer, 
			int maxiter, int iprint);
void SetScreening(int n_screen, int *screened_n, 
		  double screened_harge, int kl);
int OptimizeRadial(cfac_t *cfac, int ng, int *kg, double *weight);
int RefineRadial(int maxfun, int msglvl);
double ConfigEnergyShiftCI(int nrs0, int nrs1);
double ConfigEnergyShift(int ns, SHELL *bra, int ia, int ib, int m2);
double ConfigEnergyVariance(int ns, SHELL *bra, int ia, int ib, int m2);
int ConfigEnergy(cfac_t *cfac, int m, int mr, int ng, int *kg);
double TotalEnergyGroup(const cfac_t *cfac, int kg);
double ZerothEnergyConfig(CONFIG *cfg);
double ZerothResidualConfig(CONFIG *cfg);
double AverageEnergyConfig(CONFIG *cfg);
double AverageEnergyAvgConfig(AVERAGE_CONFIG *cfg);

/* routines for radial integral calculations */
int GetYk(int k, double *yk, ORBITAL *orb1, ORBITAL *orb2, 
	  int k1, int k2, RadIntType type);
int IntegrateF(const double *f, const ORBITAL *orb1, const ORBITAL *orb2,
    RadIntType type, double x[], int id);
int IntegrateS(const double *f, const ORBITAL *orb1, const ORBITAL *orb2, 
    RadIntType type, double *r, int id);
int IntegrateSubRegion(int i0, int i1, 
		       const double *f,
                       const ORBITAL *orb1, const ORBITAL *orb2,
		       RadIntType type, double *r, int m, int last_only);
int IntegrateSinCos(int j, double *x, double *y, 
		    double *phase, double *dphase, 
		    int i0, double *r, int last_only);
int SlaterTotal(cfac_t *cfac,
    double *sd, double *se, int *js, int *ks, int k, int mode);
double Vinti(int k0, int k1);
double QED1E(cfac_t *cfac, int k0, int k1);
double SelfEnergyRatio(ORBITAL *orb);
int Slater(double *s, int k0, int k1, int k2, int k3, int k, int mode);
double BreitC(int n, int m, int k, int k0, int k1, int k2, int k3);
double BreitS(int k0, int k1, int k2, int k3, int k);
double BreitI(int n, int k0, int k1, int k2, int k3, int m);
double Breit(int k0, int k1, int k2, int k3, int k,
	     int kl0, int kl1, int kl2, int kl3);
void SortSlaterKey(int *kd);
void PrepSlater(int ib0, int iu0, int ib1, int iu1,
		int ib2, int iu2, int ib3, int iu3);
int ResidualPotential(double *s, int k0, int k1);
double MeanPotential(int k0, int k1);
int FreeResidualArray(void);
int FreeMultipoleArray(void);
int FreeSlaterArray(void);
int FreeSimpleArray(MULTI *ma);
int FreeMomentsArray(void);
int FreeGOSArray(void);

double RadialMoments(int m, int k1, int k2);
double MultipoleRadialNR(int m, int k1, int k2, int guage);
int MultipoleRadialFRGrid(double **p, int m, int k1, int k2, int guage);
double MultipoleRadialFR(double aw, int m, int k1, int k2, int guage);
double InterpolateMultipole(double aw2, int n, double *x, double *y);
double *GeneralizedMoments(int k0, int k1, int m);
void PrintGeneralizedMoments(char *fn, int m, int n0, int k0, int n1, int k1, 
			     double e1);
int SaveOrbital(int i);
int RestoreOrbital(int i); 
int FreeOrbital(int i);
int SaveAllContinua(int mode); 
int SaveContinua(double e, int mode);
int FreeAllContinua(void);
int FreeContinua(double e);
int ClearOrbitalTable(int m);
void LimitArrayRadial(int m, double n);
int InitRadial(void);
int ReinitRadial(int m);
int TestIntegrate(void);

#endif

