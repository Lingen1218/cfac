#ifndef _TRANSITION_H_
#define _TRANSITION_H_

int SetTransitionCut(double c0, double c);
double GetTransitionCut(void);
void SetTransitionMode(int m);
void SetTransitionGauge(int m);
void SetTransitionMaxE(int m);
void SetTransitionMaxM(int m);
void SetTransitionOptions(int gauge, int mode, int max_e, int max_m);
int GetTransitionGauge(void);
int GetTransitionMode(void);
int TRMultipole(double *strength, double *energy,
		int m, int low, int up, int strict);
int OverlapLowUp(int nlow, int *low, int nup, int *up);
int SaveTransition(int nlow, int *low, int nup, int *up,
		   char *fn, int multipole);
int SaveTransitionEB(int nlow, int *low, int nup, int *up,
		     char *fn, int multipole);
int GetLowUpEB(int *nlow, int **low, int *nup, int **up, 
	       int nlow0, int *low0, int nup0, int *up0);
int PolarizeCoeff(char *ifn, char *ofn, int i0, int i1);

int TRMultipoleEB(double *strength, double *energy, int m,
                  int lower, int upper);

#endif
