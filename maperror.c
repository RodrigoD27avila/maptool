/*See Copyright Notice in map.h*/

#include "maperror.h"

char *MAP_GetErrorString(int error)
{
	switch (error) {
	case (-MAP_EALLOC):
		return "ERROR: there was a problem when allocating memory.";
	case (-MAP_ENULL):
		return "ERROR: null pointer.";
	case (-MAP_EFILE):
		return "ERROR: there is a problem with your file,"
			" maybe it does not exist.";
	case (-MAP_ELMAX):
		return "ERROR: is only possible to access 1024 layers.";
	case (-MAP_EINDEX):
		return "ERROR: trying to access an index that does not exist.";
	case (-MAP_EWRITE):
		return "ERROR: cannot write data to file.";
	case (-MAP_EREAD):
		return "ERROR: cannot read data from file.";
	case (-MAP_ESEEK):
		return "ERROR: cannot set the file position.";
	}
	return NULL;
}
