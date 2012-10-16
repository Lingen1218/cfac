#ifndef __CFACP_H_
#define __CFACP_H_

#include "cfac.h"
#include "array.h"
#include "config.h"
#include "orbital.h"
#include "radial.h"
#include "structure.h"

typedef struct {
  char symbol[5];
  double atomic_number;
  double mass;
  double rn;
} cfac_nucleus_t;

struct _cfac_t {
    unsigned int anum;
    cfac_nucleus_t nucleus;

    CONFIG_GROUP *cfg_groups; /* a list of configuration groups              */
    int n_groups;             /* number of configuration groups present      */

    double ef, bf, eb_angle;  /* electric, magnetic ield, and angle between  */
    double e1[3];             /* spherical components of the E field         */
    double b0, b1[3], b2[5];  /* 0th, 1st, and 2nd-order tensors of B        */

    SYMMETRY *symmetry_list;  /* list of symmetries. The i-th symmetry has
                                 j = floor(i/2) and parity = mod(i, 2).      */

    ARRAY *levels_per_ion;

    POTENTIAL *potential;     /* potential                                   */


    ARRAY *orbitals;          /* array of orbitals                           */
    int n_orbitals;           /* total number of orbitals                    */
    int n_continua;           /* number of continuum orbitals                */
 
    MULTI *slater_array;
    MULTI *breit_array;
    MULTI *vinti_array;
    MULTI *qed1e_array;
    MULTI *residual_array;
    MULTI *multipole_array; 
    MULTI *moments_array;
    MULTI *gos_array;
    MULTI *yk_array;

    struct {
      double stabilizer;
      double tolerance;       /* tolerance for self-consistency              */
      int maxiter;            /* max iter. for self-consistency              */
      double screened_charge; 
      int screened_kl;
      int n_screen;
      int *screened_n;
      int iprint;             /* printing infomation in each iteration.      */
      int iset;
    } optimize_control;
    
    struct {
      int kl0;
      int kl1;
    } slater_cut;

    struct {
      int se;
      int vp;
      int nms;
      int sms;
      int br;
    } qed;

    AVERAGE_CONFIG average_config;

    int n_awgrid;
    double awgrid[MAXNTE];

    
    HAMILTON *hamiltonian;    /* Hamiltonian                                 */

    SHAMILTON *hams;          /* state Hamiltonians                          */
    int nhams;                /* number of them in use                       */

    ARRAY *levels;            /* levels                                      */
    int n_levels;             /* number of levels                            */
    
    ARRAY *eblevels;          /* levels when calculated with fields          */
    int n_eblevels;           /* number of eblevels                          */

    ARRAY *ecorrections;      /* energy corrections                          */
    int ncorrections;         /* number of energy corrections                */

    int confint;              /* configuration interaction flag (-1,0,1,2,3) */

    int angz_maxn;            /* max PQN above which angular mix between
                                 different bound configurations is ignored   */
    double angz_cut;          /* threshold of angular mixing                 */
    
    double mix_cut;           /* threshold mixing (relative to the leading
                                 component); bases with weaker mixings are
                                 not recoupled                               */
    
    double mix_cut2;          /* _lower_ threshold for recoupling between
                                 different configurations                    */

    ANGZ_DATUM *angz_array;   /* angular coefficients                        */
    ANGZ_DATUM *angzxz_array; /* ZxZ angular coefficients                    */
    ANGZ_DATUM *angmz_array;  /* precalculated angular coefficients          */

    ANGULAR_FROZEN ang_frozen;/* angular coefficients for frozen states      */
};

/* config.c */
void
FreeConfigData(void *p);
void
InitConfigData(void *p, int n);

/* radial.c */
void
FreeOrbitalData(void *p);
void
InitOrbitalData(void *p, int n);
void
FreeMultipole(void *p);
void
FreeYkData(void *p);
void
InitYkData(void *p, int n);

/* structure.c */
void
FreeLevelData(void *p);
void
FreeHamsArray(cfac_t *cfac);
void
FreeAngZDatum(ANGZ_DATUM *ap);
void
InitLevelData(void *p, int n);

#endif /* __CFACP_H_ */
