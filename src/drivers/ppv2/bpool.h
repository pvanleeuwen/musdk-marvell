/**************************************************************************//**
	Copyright (C) 2016 Marvell International Ltd.
*//***************************************************************************/

#ifndef __BPOOL_H__
#define __BPOOL_H__


#include "drivers/mv_pp2_bpool.h"

struct pp2_bpool {
	int id; /* BM Pool number */
	struct base_addr *pp2_hw_base;
};



#endif /* __BPOOL_H__ */