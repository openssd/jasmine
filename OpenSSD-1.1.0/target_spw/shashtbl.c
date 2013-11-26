// Copyright 2011 INDILINX Co., Ltd.
//
// This file is part of Jasmine.
//
// Jasmine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Jasmine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Jasmine. See the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.
//
//
// Lightweight static hash table source file
//

#include "jasmine.h"

SHASHTBL g_shashtbl[NUM_BANKS];

/**
 * Hash function : Robert Jenkins' 32 bit integer hash
 *
 * @param key    : key value (logical page number of user data)
 *
 * @return : hash value
 */
static UINT32 hashfunc_RJ(UINT32 const key)
{
    UINT32 hash = key;

    hash = (hash + 0x7ed55d16) + (hash << 12);
    hash = (hash ^ 0xc761c23c) ^ (hash >> 19);
    hash = (hash + 0x165667b1) + (hash << 5);
    hash = (hash + 0xd3a2646c) ^ (hash << 9);
    hash = (hash + 0xfd7046c5) + (hash << 3);
    hash = (hash ^ 0xb55a4f09) ^ (hash >> 16);

    return hash;
}
void shashtbl_init(void)
{
    for (UINT32 bank = 0; bank < NUM_BANKS; bank++) {
        g_shashtbl[bank].num_elmts = 0;
        g_shashtbl[bank].free_node_pool = NULL;
    }
    mem_set_dram(HASH_NODE_ADDR, NULL, HASH_NODE_BYTES);
    mem_set_dram(HASH_BUCKET_ADDR, NULL, HASH_BUCKET_BYTES);
}
void shashtbl_insert(UINT32 const bank, UINT32 const key, UINT32 const data)
{
    hashnode_ptr new_node;
    UINT32 node_idx = g_shashtbl[bank].num_elmts;
	UINT32 hash_val = hashfunc_RJ(key) % HASH_BUCKET_SIZE;

    ASSERT(shashtbl_get(bank, key) == INVALID);
    ASSERT(node_idx < (LOG_BLK_PER_BANK * PAGES_PER_BANK));
    ASSERT(data < (LOG_BLK_PER_BANK * PAGES_PER_BANK));

    if (g_shashtbl[bank].free_node_pool == NULL) {
        ASSERT(g_shashtbl[bank].num_elmts < (LOG_BLK_PER_BANK * PAGES_PER_BANK));
        new_node = (hashnode_ptr)(HASH_NODE_ADDR + ((bank * (LOG_BLK_PER_BANK + ISOL_BLK_PER_BANK) * PAGES_PER_BLK) + node_idx) * sizeof(hashnode));

        g_shashtbl[bank].num_elmts++;
    }
    else {
        new_node = g_shashtbl[bank].free_node_pool;
        g_shashtbl[bank].free_node_pool = (hashnode_ptr)read_dram_32(&new_node->next);
    }
    ASSERT((UINT32)new_node > DRAM_BASE);
    // dangling to bucket head
    hashnode_ptr tnodeptr = (hashnode_ptr)read_dram_32(HASH_BUCKET_ADDR + ((bank * HASH_BUCKET_SIZE) + hash_val) * sizeof(hashnode_ptr));
    hashnode temp_node = {key, data, tnodeptr};

    // assign key & data to new node and dangling specific hash bucket
    mem_copy(new_node, &temp_node, sizeof(hashnode));
    write_dram_32(HASH_BUCKET_ADDR + (((bank * HASH_BUCKET_SIZE) + hash_val) * sizeof(hashnode_ptr)), new_node);
}
UINT32 shashtbl_get(UINT32 const bank, UINT32 const key)
{
	hashnode_ptr node;
	UINT32 hash_val;

    hash_val = hashfunc_RJ(key) % HASH_BUCKET_SIZE;
	node = (hashnode_ptr)read_dram_32(HASH_BUCKET_ADDR + ((bank * HASH_BUCKET_SIZE + hash_val) * sizeof(hashnode_ptr)));

	while(node) {
		if(read_dram_32(node) == key) {
            return read_dram_32(&node->data); // return log_lpn
        }
		node = (hashnode_ptr)read_dram_32(&node->next);
	}
    return INVALID;
}
void shashtbl_remove(UINT32 const bank, UINT32 const key)
{
	hashnode_ptr node;
    hashnode_ptr prevnode = NULL;
    UINT32 hash_val;

    hash_val = hashfunc_RJ(key) % HASH_BUCKET_SIZE;
    node = (hashnode_ptr)read_dram_32(HASH_BUCKET_ADDR + ((bank * HASH_BUCKET_SIZE + hash_val) * sizeof(hashnode_ptr)));

    // search the node to remove as comparing key value
	while(node) {
		if(read_dram_32(node) == key) {
			if(prevnode) {
                write_dram_32(&prevnode->next, read_dram_32(&node->next));
            }
			else {
                hashnode_ptr tnode = (hashnode_ptr)read_dram_32(&node->next);
                write_dram_32(HASH_BUCKET_ADDR + ((bank * HASH_BUCKET_SIZE + hash_val) * sizeof(hashnode_ptr)), (UINT32)tnode);
            }
            // push free node to free_node_pool
            write_dram_32(&node->next, g_shashtbl[bank].free_node_pool);
            g_shashtbl[bank].free_node_pool = node;
            ASSERT(shashtbl_get(bank, key) == INVALID);
            return;
        }
		prevnode = node;
		node = (hashnode_ptr)read_dram_32(&node->next);
	}
    ASSERT(0);
}
void shashtbl_update(UINT32 const bank, UINT32 const key, UINT32 const new_val)
{
    ASSERT(new_val != INVALID);

	hashnode_ptr node;
    UINT32 hash_val;
    #if OPTION_ENABLE_ASSERT
    UINT32 find_flag = FALSE;
    #endif
    hash_val = hashfunc_RJ(key) % HASH_BUCKET_SIZE;
    node = (hashnode_ptr)read_dram_32(HASH_BUCKET_ADDR + ((bank * HASH_BUCKET_SIZE + hash_val) * sizeof(hashnode_ptr)));

    /// search the node to remove as comparing key value
	while(node) {
		if(read_dram_32(node) == key) {
            // update as a new value
            write_dram_32(&node->data, new_val);
            #if OPTION_ENABLE_ASSERT
            find_flag = TRUE;
            #endif
            break;
        }
		node = (hashnode_ptr)read_dram_32(&node->next);
	}
    ASSERT(find_flag == TRUE);
}
