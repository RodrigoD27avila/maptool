/*See Copyright Notice in map.h*/

#ifndef MAPERROR_H_INCLUDED
#define MAPERROR_H_INCLUDED

#include <stdlib.h>

#define MAP_NO_ERROR 0
#define MAP_EALLOC   1
#define MAP_ENULL    2
#define MAP_EFILE    3
#define MAP_ELMAX    4
#define MAP_EINDEX   5
#define MAP_EWRITE   6
#define MAP_EREAD    7
#define MAP_ESEEK    8

#ifdef __cplusplus
extern "C" {
#endif

char *MAP_GetErrorString(int error);

#ifdef __cplusplus
}
#endif

#endif /*MAPERROR_H_INCLUDED*/
