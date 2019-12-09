// Added by George Marselis <george@marsel.is> Thu Dec 5 2019

#include "lib/getGrpData.h"

// came from libh/liblsf/lsb_params.c
struct groupInfoEnt *
getGrpData (struct groupInfoEnt **groups, const char *groupname, size_t ngroups)
{
    if( NULL == groupname || NULL == groups ) {
        return NULL;
    }

    for( size_t = 0; i < ngroups; i++ ) {
        if (groups[i] && groups[i]->group && (strcmp (groupname, groups[i]->group) == 0) ) {
            return groups[i];
        }
    }

    return NULL;
}


// came from daemons/mbatchd/grp.c
// struct gData *
// getGrpData (struct gData *groups[], const char *groupname, unsigned int num)
// {
//     if( NULL == groups || NULL == groupname ) {
//         return NULL;
//     }

//     for ( unsigned int i = 0; i < num; i++) {
//         if( strcmp( name, groups[i]->group ) == 0 )
//             return groups[i];
//     }

//     return NULL;
// }
