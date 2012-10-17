#include <stdio.h>

#include <gsl/gsl_ieee_utils.h>

#include "cfacP.h"
#include "global.h"
#include "coulomb.h"
#include "recouple.h"
#include "angular.h"
#include "radial.h"
#include "excitation.h"
#include "ionization.h"
#include "recombination.h"
#include "dbase.h"
#include "init.h"

cfac_t *cfac = NULL;

int Info(void) {
  printf("========================================\n");
  printf("The Flexible Atomic Code (FAC)\n");
  printf("Version %d.%d.%d\n", VERSION, SUBVERSION, SUBSUBVERSION);
  printf("Bugs and suggestions, please contact:\n");
  printf("Ming Feng Gu, mfgu@ssl.berkeley.edu\n");
  printf("========================================\n");
  return 0;
}

int InitFac() {
  gsl_ieee_env_setup();

  cfac = cfac_new();
  if (!cfac) {
    printf("Initialization failed\n");
    return -1;
  }

  InitCoulomb();
  InitAngular();

  InitDBase();
  InitExcitation();
  InitRecombination();
  InitIonization();
  
  return 0;
}
