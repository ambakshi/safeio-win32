#ifndef SAFEIO_H
#define SAFEIO_H

#include <windows.h>
#include <stdio.h>

DWORD sfrename(FILE *fp, const char *path);
FILE *sfopen(const char *path, const char *mode);

#endif
