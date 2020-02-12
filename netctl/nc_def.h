#ifndef __NETDEF_H
#define __NETDEF_H
struct nc_distort {
    unsigned int old_addr;
    unsigned int new_addr;
    unsigned short old_port;
    unsigned short new_port;
};
#define procfs_name "netctl"

#endif