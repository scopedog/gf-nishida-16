#ifndef _AES_GCM_H_
#define _AES_GCM_H_

#include <stdint.h>


/************************************************************
	Definitions
************************************************************/


/************************************************************
	Variables
************************************************************/


/************************************************************
	Functions
************************************************************/

void		gf_mult(const uint8_t *, const uint8_t *, uint8_t *); 

#endif // _AES_GCM_H_