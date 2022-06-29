#include "public/ingenialink/version.h"

/*******************************************************************************
 * Public
 ******************************************************************************/

const char *il_version()
{
	return IL_VERSION;
}


void free_(void *pointer) 
{
	free(pointer);
}