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

/**
 * @file pp2_cls_db.c
 *
 * access routines to pp2_cls_db
 */

/***********************/
/* c file declarations */
/***********************/
#include "std_internal.h"

#include "../pp2_types.h"
#include "../pp2.h"
#include "../pp2_print.h"
#include "../pp2_hw_type.h"
#include "../pp2_hw_cls.h"

/* Global PP2_CLS database */
static struct pp2_cls_db_t *g_pp2_cls_db;

/*******************************************************************************
* pp2_cls_db_mem_alloc_init
*
* DESCRIPTION: The routine will allcate mem for pp2_cls db and init it.
*
* INPUTS:
*	None.
*
* OUTPUTS:
*	None
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_error_code_t.
*
* COMMENTS:
*	None
*******************************************************************************/
static int pp2_cls_db_mem_alloc_init(void)
{
	/* Allocation for g_pp2_cls_db */
	g_pp2_cls_db = malloc(sizeof(struct pp2_cls_db_t));
	if (!g_pp2_cls_db)
		goto fail1;

	/* Erase DB */
	MVPP2_MEMSET_ZERO(*g_pp2_cls_db);

	return 0;

fail1:
	pp2_err("PP2_CLS DB memory allocation failed\n");
	return -ENOMEM;
}

/*******************************************************************************
* pp2_cls_db_mem_free
*
* DESCRIPTION: The routine will free memory for allocated for pp2_cls db.
*
* INPUTS:
*	None.
*
* OUTPUTS:
*	None
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_error_code_t.
*
* COMMENTS:
*	None
*******************************************************************************/
static int pp2_cls_db_mem_free(void)
{
	free(g_pp2_cls_db);

	return 0;
}

/*******************************************************************************
* pp2_cls_db_module_state_set
*
* DESCRIPTION: The API sets PP2_CLS module init state, either not started, or started
*
* INPUTS:
*	state  - PP2_CLS module init state
*
* OUTPUTS:
*	None.
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_error_code_t.
*
* COMMENTS:
*	None
*******************************************************************************/
int pp2_cls_db_module_state_set(enum pp2_cls_module_state_t state)
{
	if (state > MVPP2_MODULE_STARTED) {
		pp2_err("Invalid state(%d)\n", state);
		return -EBUSY;
	}

	g_pp2_cls_db->pp2_cls_module_init_state = state;

	return 0;
}

/*******************************************************************************
* pp2_cls_db_module_state_get
*
* DESCRIPTION: The API gets PP2_CLS module init state, either not started, or started
*
* INPUTS:
*	None.
*
* OUTPUTS:
*	None.
*
* RETURNS:
*	PP2_CLS module init state
*
* COMMENTS:
*	None
*******************************************************************************/
u32 pp2_cls_db_module_state_get(void)
{
	return g_pp2_cls_db->pp2_cls_module_init_state;
}

/*******************************************************************************
* pp2_cls_db_c3_free_logic_idx_get()
*
* DESCRIPTION: Get a free logic index from list.
*
* INPUTS:
*	None
*
* OUTPUTS:
*	logic_idx - logical index
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*******************************************************************************/
int pp2_cls_db_c3_free_logic_idx_get(u32 *logic_idx)
{
	int idx;

	if (mv_pp2x_ptr_validate(logic_idx))
		return -EINVAL;

	/* search for valid C3 logical index */
	for (idx = 0; idx < MVPP2_CLS_C3_HASH_TBL_SIZE; idx++) {
		if (g_pp2_cls_db->c3_db.hash_idx_tbl[idx].valid == MVPP2_C3_ENTRY_INVALID)
			break;
	}

	if (idx >= MVPP2_CLS_C3_HASH_TBL_SIZE) {
		*logic_idx = 0;
		return -EIO;
	}

	*logic_idx = idx;

	return 0;
}

/*******************************************************************************
* pp2_cls_db_c3_entry_add()
*
* DESCRIPTION: Add C3 entry to DB.
*
* INPUTS:
*	logic_idx      - logical index
*	hash_idx       - multihash index
*
* OUTPUTS:
*	None
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*******************************************************************************/
int pp2_cls_db_c3_entry_add(u32 logic_idx, u32 hash_idx)
{
	if (mv_pp2x_range_validate(logic_idx, 0, MVPP2_CLS_C3_HASH_TBL_SIZE - 1))
		return -EINVAL;

	if (mv_pp2x_range_validate(hash_idx, 0, MVPP2_CLS_C3_HASH_TBL_SIZE - 1))
		return -EINVAL;

	/* add or update hash index table */
	g_pp2_cls_db->c3_db.hash_idx_tbl[logic_idx].valid = MVPP2_C3_ENTRY_VALID;
	g_pp2_cls_db->c3_db.hash_idx_tbl[logic_idx].hash_idx = hash_idx;

	/* add or update logical index table */
	g_pp2_cls_db->c3_db.logic_idx_tbl[hash_idx].valid = MVPP2_C3_ENTRY_VALID;
	g_pp2_cls_db->c3_db.logic_idx_tbl[hash_idx].logic_idx = logic_idx;

	return 0;
}

/*******************************************************************************
* pp2_cls_db_c3_entry_del()
*
* DESCRIPTION: Delete C3 entry to DB.
*
* INPUTS:
*	logic_idx - logical index
*
* OUTPUTS:
*	None
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*******************************************************************************/
int pp2_cls_db_c3_entry_del(int logic_idx)
{
	u32 hash_idx;
	struct pp2_cls_c3_hash_index_entry_t *p_hash_entry = NULL;
	struct pp2_cls_c3_logic_index_entry_t *p_logic_entry = NULL;

	if (mv_pp2x_range_validate(logic_idx, 0, MVPP2_CLS_C3_HASH_TBL_SIZE - 1))
		return -EINVAL;

	/* get hash index entry */
	p_hash_entry = &g_pp2_cls_db->c3_db.hash_idx_tbl[logic_idx];
	if (p_hash_entry->valid == MVPP2_C3_ENTRY_VALID) {
		/* clear hash entry */
		hash_idx = p_hash_entry->hash_idx;
		p_hash_entry->valid = MVPP2_C3_ENTRY_INVALID;
		p_hash_entry->hash_idx = MVPP2_C3_INVALID_ENTRY_NUM;

		/* get logical index entry by hash index and clear it */
		if (mv_pp2x_range_validate(hash_idx, 0, MVPP2_CLS_C3_HASH_TBL_SIZE - 1))
			return -EINVAL;
		p_logic_entry = &g_pp2_cls_db->c3_db.logic_idx_tbl[hash_idx];
		p_logic_entry->valid = MVPP2_C3_ENTRY_INVALID;
		p_logic_entry->logic_idx = MVPP2_C3_INVALID_ENTRY_NUM;
	} else {
		pp2_err("hash entry is invalid, do not need to delete it\n");
	}

	return 0;
}

/*******************************************************************************
* pp2_cls_db_c3_hash_idx_get()
*
* DESCRIPTION: Get C3 multihash index by logical index.
*
* INPUTS:
*	logic_idx - logical index
*
* OUTPUTS:
*	hash_idx  - multihash index
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*******************************************************************************/
int pp2_cls_db_c3_hash_idx_get(u32 logic_idx, u32 *hash_idx)
{
	struct pp2_cls_c3_hash_index_entry_t *p_hash_entry = NULL;

	if (mv_pp2x_range_validate(logic_idx, 0, MVPP2_CLS_C3_HASH_TBL_SIZE - 1))
		return -EINVAL;

	if (mv_pp2x_ptr_validate(hash_idx))
		return -EINVAL;

	/* get hash index entry */
	p_hash_entry = &g_pp2_cls_db->c3_db.hash_idx_tbl[logic_idx];
	if (p_hash_entry->valid == MVPP2_C3_ENTRY_VALID) {
		*hash_idx = p_hash_entry->hash_idx;
		return 0;
	} else {
		return -EIO;
	}
	return 0;
}

/*******************************************************************************
* pp2_cls_db_c3_logic_idx_get()
*
* DESCRIPTION: Get the multihash index.
*
* INPUTS:
*	hash_idx  - multihash index
*
* OUTPUTS:
*	logic_idx - logical index
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*******************************************************************************/
int pp2_cls_db_c3_logic_idx_get(int hash_idx, int *logic_idx)
{
	struct pp2_cls_c3_logic_index_entry_t *p_logic_entry = NULL;

	if (mv_pp2x_range_validate(hash_idx, 0, MVPP2_CLS_C3_HASH_TBL_SIZE - 1))
		return -EINVAL;

	if (mv_pp2x_ptr_validate(logic_idx))
		return -EINVAL;

	/* get hash index entry */
	p_logic_entry = &g_pp2_cls_db->c3_db.logic_idx_tbl[hash_idx];
	if (p_logic_entry->valid == MVPP2_C3_ENTRY_VALID) {
		*logic_idx = p_logic_entry->logic_idx;
		return 0;
	} else {
		return -EIO;
	}

	return 0;
}

/*******************************************************************************
* pp2_cls_db_c3_hash_idx_update()
*
* DESCRIPTION: Update the multihash index.
*
* INPUTS:
*	hash_pair_arr  - multihash modification array
*
* OUTPUTS:
*	None
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*******************************************************************************/
int pp2_cls_db_c3_hash_idx_update(struct pp2_cls_c3_hash_pair *hash_pair_arr)
{
	int idx;
	u32 old_idx;
	u32 new_idx;
	u32 logic_idx;
	struct pp2_cls_c3_hash_index_entry_t *p_hash_entry = NULL;
	struct pp2_cls_c3_logic_index_entry_t *p_logic_entry = NULL;

	if (mv_pp2x_ptr_validate(hash_pair_arr))
		return -EINVAL;

	/* update the multihash mapping in loop */
	for (idx = 0; idx < hash_pair_arr->pair_num; idx++) {
		old_idx = hash_pair_arr->old_idx[idx];
		new_idx = hash_pair_arr->new_idx[idx];

		if (mv_pp2x_range_validate(old_idx, 0, MVPP2_CLS_C3_HASH_TBL_SIZE - 1))
			return -EINVAL;

		if (mv_pp2x_range_validate(new_idx, 0, MVPP2_CLS_C3_HASH_TBL_SIZE - 1))
			return -EINVAL;

		p_logic_entry = &g_pp2_cls_db->c3_db.logic_idx_tbl[old_idx];
		if (p_logic_entry->valid == MVPP2_C3_ENTRY_INVALID) {
			pp2_err("hash entry is invalid w/ index(%d)\n", old_idx);
			return MV_ERROR;
		}

		logic_idx = p_logic_entry->logic_idx;

		if (mv_pp2x_range_validate(logic_idx, 0, MVPP2_CLS_C3_HASH_TBL_SIZE - 1))
			return -EINVAL;

		/* update logical index table */
		p_logic_entry->valid = MVPP2_C3_ENTRY_INVALID;
		p_logic_entry->logic_idx = MVPP2_C3_INVALID_ENTRY_NUM;
		p_logic_entry = &g_pp2_cls_db->c3_db.logic_idx_tbl[new_idx];
		p_logic_entry->valid = MVPP2_C3_ENTRY_VALID;
		p_logic_entry->logic_idx = logic_idx;

		/* update hash index table */
		p_hash_entry = &g_pp2_cls_db->c3_db.hash_idx_tbl[logic_idx];
		p_hash_entry->valid = MVPP2_C3_ENTRY_VALID;
		p_hash_entry->hash_idx = new_idx;
	}

	return 0;
}

/*******************************************************************************
* pp2_cls_db_c3_scan_param_set()
*
* DESCRIPTION: set scan parameters.
*
* INPUTS:
*	scan_config  - scan configuration parameters
*
* OUTPUTS:
*	None
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*******************************************************************************/
int pp2_cls_db_c3_scan_param_set(struct pp2_cls_c3_scan_config_t *scan_config)
{
	struct pp2_cls_c3_scan_config_t *p_scan_config = NULL;

	if (mv_pp2x_ptr_validate(scan_config))
		return -EINVAL;

	p_scan_config = &g_pp2_cls_db->c3_db.scan_config;
	memcpy(p_scan_config, scan_config, sizeof(struct pp2_cls_c3_scan_config_t));

	return 0;
}

/*******************************************************************************
* pp2_cls_db_c3_scan_param_get()
*
* DESCRIPTION: get scan parameters.
*
* INPUTS:
*	Nones
*
* OUTPUTS:
*	can_config  - scan configuration parameters
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*******************************************************************************/
int pp2_cls_db_c3_scan_param_get(struct pp2_cls_c3_scan_config_t *scan_config)
{
	struct pp2_cls_c3_scan_config_t *p_scan_config = NULL;

	if (mv_pp2x_ptr_validate(scan_config))
		return -EINVAL;

	p_scan_config = &g_pp2_cls_db->c3_db.scan_config;
	memcpy(scan_config, p_scan_config, sizeof(struct pp2_cls_c3_scan_config_t));

	return 0;
}

/*******************************************************************************
* pp2_cls_db_c3_search_depth_set()
*
* DESCRIPTION: set cuckoo search depth.
*
* INPUTS:
*	search_depth  - cuckoo search depth
*
* OUTPUTS:
*	None
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*******************************************************************************/
int pp2_cls_db_c3_search_depth_set(u32 search_depth)
{
	g_pp2_cls_db->c3_db.max_search_depth = search_depth;
	return 0;
}

/*******************************************************************************
* pp2_cls_db_c3_search_depth_get()
*
* DESCRIPTION: get cuckoo search depth.
*
* INPUTS:
*	None
*
* OUTPUTS:
*	search_depth  - cuckoo search depth
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*******************************************************************************/
int pp2_cls_db_c3_search_depth_get(u32 *search_depth)
{
	if (mv_pp2x_ptr_validate(search_depth))
		return -EINVAL;

	*search_depth = g_pp2_cls_db->c3_db.max_search_depth;

	return 0;
}

/*******************************************************************************
* pp2_cls_db_c3_init()
*
* DESCRIPTION: Perform DB Initialization for C3 engine.
*
* INPUTS:
*	None.
*
* OUTPUTS:
*	None.
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_error_code_t.
*******************************************************************************/
int pp2_cls_db_c3_init(void)
{
	int idx;

	if (!g_pp2_cls_db)
		return -EINVAL;

	/* Clear C3 db */
	memset(&g_pp2_cls_db->c3_db, 0, sizeof(struct pp2_cls_db_c3_t));

	/* Init C3 multihash index table and logical index table */
	for (idx = 0; idx < MVPP2_CLS_C3_HASH_TBL_SIZE; idx++) {
		g_pp2_cls_db->c3_db.hash_idx_tbl[idx].valid = MVPP2_C3_ENTRY_INVALID;
		g_pp2_cls_db->c3_db.logic_idx_tbl[idx].valid = MVPP2_C3_ENTRY_INVALID;
	}

	return 0;
}

/*******************************************************************************
* pp2_cls_db_init()
*
* DESCRIPTION: Perform DB Initialization.
*
* INPUTS:
*	None.
*
* OUTPUTS:
*	None.
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int pp2_cls_db_init(void)
{
	int ret_code;

	/* Allocation for pp2_cls db */
	ret_code = pp2_cls_db_mem_alloc_init();

	if (ret_code != 0) {
		pp2_err("Failed to allocate memory for PP2_CLS DB\n");
		return -ENOMEM;
	}

	/* Set PP2_CLS module state */
	ret_code = pp2_cls_db_module_state_set(MVPP2_MODULE_NOT_START);
	if (ret_code != 0) {
		pp2_err("Failed to set PP2_CLS module stat\n");
		return -EINVAL;
	}

	return 0;
}

/*******************************************************************************
* pp2_cls_db_exit()
*
* DESCRIPTION: Perform DB memory free when exit.
*
* INPUTS:
*	None.
*
* OUTPUTS:
*	None.
*
* RETURNS:
*	On success, the function returns PP2_CLS_OK. On error different types are returned
*	according to the case - see pp2_cls_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int pp2_cls_db_exit(void)
{
	int ret_code;

	ret_code = pp2_cls_db_mem_free();
	if (ret_code != 0) {
		pp2_err("Failed to free memory allocated for PP2_CLS DB\n");
		return -ENOMEM;
	}

	return 0;
}