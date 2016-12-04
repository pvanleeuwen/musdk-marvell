/******************************************************************************
 *	Copyright (C) 2016 Marvell International Ltd.
 *
 *  If you received this File from Marvell, you may opt to use, redistribute
 *  and/or modify this File under the following licensing terms.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *
 *	* Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 *
 *	* Neither the name of Marvell nor the names of its contributors may be
 *	  used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef __MV_PP2_BPOOL_H__
#define __MV_PP2_BPOOL_H__

#include "mv_std.h"
#include "mv_pp2_hif.h"

/** @addtogroup grp_pp2_bp Packet Processor: Buffer Pool
 *  
 *  Packet Processor Buffer Pool API documentation
 *
 *  @{
 */

struct pp2_bpool;


struct pp2_bpool_params {
	/** Used for DTS acc to find appropriate "physical" BM-Pool obj;
	 * E.g. "pool-0:0" means PPv2[0],pool[0] */
	const char			*match;
	u32				 max_num_buffs;
	u32				 buff_len;
/* TODO: will not be supported at first stage. Need to look how to handle HW IRQ
	int				 (*empty_cb) (void *arg, u32 status);
	void				 *emty_cb_arg;
	u16				 threashold_hi;
	u16				 threashold_lo;
*/
};


int pp2_bpool_init(struct pp2_bpool_params *params, struct pp2_bpool **bpool);

void pp2_bpool_deinit(struct pp2_bpool *bpool);


#ifdef CONF_PP2_BPOOL_DMA_ADDR_USE_32B
typedef u32 	bpool_dma_addr_t;
#else
typedef dma_addr_t bpool_dma_addr_t;
#endif


#ifdef CONF_PP2_BPOOL_COOKIE_SIZE
#if CONF_PP2_BPOOL_COOKIE_SIZE == 64
typedef u64 	pp2_cookie_t;
#else
typedef u32	pp2_cookie_t;
#endif
#endif

struct pp2_buff_inf {
	/**< Note: in 64bits systems and user would like to use only 32bits,
	 * use CONF_PP2_BPOOL_DMA_ADDR_USE_32B */
	bpool_dma_addr_t addr;
#ifdef CONF_PP2_BPOOL_COOKIE_SIZE
	pp2_cookie_t    cookie;
#endif
};


int pp2_bpool_get_buff(struct pp2_hif *hif, struct pp2_bpool *pool, struct pp2_buff_inf *buff);
int pp2_bpool_put_buff(struct pp2_hif *hif, struct pp2_bpool *pool, struct pp2_buff_inf *buff);

int pp2_bpool_get_num_buffs(struct pp2_bpool *pool, u32 *num_buffs);

/** @} */ // end of grp_pp2_bp

#endif /* __MV_PP2_BPOOL_H__ */
