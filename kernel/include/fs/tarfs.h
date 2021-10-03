#ifndef FILE_TARFS_H
#define FILE_TARFS_H
#include <stddef.h>

struct tar_header {
   char filename[100];
   char mode[8];
   char uid[8];
   char gid[8];
   char size[12];
   char mtime[12];
   char chksum[8];
   char type;
   char linked_name[100];
   char ustar[6];
   char ustar_ver[2];
   char uname[32];
   char gname[32];
   char major[8];
   char minor[8];
   char prefix[155];
};

// Finds an entry in the tar archive with the name `filename`
// and sets `*out` to it's data and sets `*size` to it's size.
//
// Parameters:
// - archive   - Pointer to the tar archive
// - filename  - Name of the file to search for
// - out       - Pointer to store the data pointer
// - size      - Pointer to store the size
//
// Returns:
// <0          - Error occured
// 0           - OK
int tar_lookup(const void* archive, const char* filename, const void** out, size_t* size);

#endif /* FILE_TARFS_H */
