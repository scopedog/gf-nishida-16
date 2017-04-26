#ifndef _GF_H_
#define _GF_H_

#include <stdint.h>


/************************************************************
	Definitions
************************************************************/

typedef uint8_t		GF8type;
typedef uint16_t	GF16type;

/************************************************************
	Variables
************************************************************/

#ifdef _GF_MAIN_
GF8type		**GF8memMul = NULL;
GF8type		**GF8memDiv = NULL;
GF16type	*GF16memOrg = NULL, *GF16memOrgH = NULL;
int		*GF16memIdx = NULL;

#else
extern GF8type	**GF8memMul;
extern GF8type	**GF8memDiv;
extern GF16type	*GF16memOrg, *GF16memOrgH;
extern int	*GF16memIdx;
#endif


/************************************************************
	Functions
************************************************************/

void	GF8init(void); 
#define	GF8mul(a, b)	(GF8memMul[(a)][(b)])
#define	GF8div(a, b)	(GF8memDiv[(a)][(b)])

void		GF16init(void); 
#if 1
//#define	GF16mul(a, b)	((!a || !b) ? 0 : GF16memOrg[GF16memIdx[(a)] + GF16memIdx[(b)]])
#define	GF16mul(a, b)	(GF16memOrg[GF16memIdx[(a)] + GF16memIdx[(b)]])
#define	GF16div(a, b)	((!a || !b) ? 0 : GF16memOrgH[GF16memIdx[(a)] - GF16memIdx[(b)]])

#else
GF16type	GF16mul(GF16type, GF16type); 
GF16type	GF16div(GF16type, GF16type); 
#endif

void		GF8test(void);
void		GF16test(void);


#endif // _GF_H_
