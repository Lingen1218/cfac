#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cfacdb.h"

typedef struct {
    unsigned int nsessions;
    unsigned long *sids;
    unsigned long nsid;
} cfacdbu_t;


static void sessions_sink(const cfacdb_t *cdb,
    cfacdb_sessions_data_t *cbdata, void *udata)
{
    cfacdbu_t *cdu = udata;
    
    cdu->nsid++;
    
    printf("Session #%lu (sid = %ld):\n",
	      cdu->nsid, cbdata->sid);
    printf("\t%s (Z = %d, mass = %.2f) nele = %d ... %d \n",
	      cbdata->sym, cbdata->anum, cbdata->mass,
              cbdata->nele_min, cbdata->nele_max);
}

int main(int argc, const char *argv[])
{
    const char *fname;
    unsigned int nele_min = 0, nele_max = 100;
    
    cfacdb_t *cdb;
    cfacdbu_t cdu;
    cfacdb_stats_t stats;
    
    memset(&cdu, 0, sizeof(cdu));
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file.db>\n", argv[0]);
        exit(1);
    }
    fname = argv[1];

    cdb = cfacdb_open(fname);
    if (!cdb) {
        fprintf(stderr, "Failed to open DB \"%s\"\n", fname);
        exit(1);
    }
    
    cdu.nsessions = cfacdb_get_nsessions(cdb);
    printf("%s: %d session(s)\n", fname, cdu.nsessions);
    
    cdu.sids = malloc(cdu.nsessions*sizeof(unsigned long));

    cfacdb_sessions(cdb, sessions_sink, &cdu);
    
    if (cfacdb_init(cdb, 0, nele_min, nele_max) != CFACDB_SUCCESS) {
        fprintf(stderr, "Initialization of DB \"%s\" failed\n", fname);
        exit(1);
    }
    
    if (cfacdb_get_stats(cdb, &stats) != CFACDB_SUCCESS) {
        fprintf(stderr, "Failed getting statistics of DB \"%s\"\n", fname);
        exit(1);
    }
    
    printf("Levels: %lu, RT: %lu, AI: %lu, CE: %lu, CI: %lu, RR: %lu\n",
        stats.ndim, stats.rtdim, stats.aidim, stats.cedim, stats.cidim,
        stats.pidim);

    cfacdb_close(cdb);
    
    exit(0);
}
