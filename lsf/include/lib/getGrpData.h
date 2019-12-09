// // Added by George Marselis <george@marsel.is> Fri December 6 2019

#pragma once



// compare the two structs above to see which one is more convinient

// struct groupInfoEnt // v1
// {
//     const char *group;
//     const char *memberList;
// };

struct groupInfoEnt // v2
{
    char *group;
    char *memberList;
    struct hTab memberTab;
    unsigned int numGroups;
    struct groupInfoEnt *groupInfoEnt[MAX_GROUPS];
}__attribute__((packed));

// equivalent to struct groupInfoEnt
// struct gData
// {
//     char *group;
//     struct hTab memberTab;
//     int numGroups;
//     struct gData *gPtr[MAX_GROUPS];
// };
