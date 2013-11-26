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
// Lightweight static hash table header file
//

#ifndef _SHASHTBL_H
#define _SHASHTBL_H

// hash node
typedef struct _hashnode {
    UINT32 key;
    UINT32 data;
    struct _hashnode* next;
}hashnode; // 12Byte node
typedef hashnode* hashnode_ptr;

// static hash table
typedef struct _shashtbl {
    UINT32 num_elmts; // the number of elements which are inserted a specific hash bucket
    hashnode_ptr free_node_pool;
}SHASHTBL;

//////////////////////////
// hash table API
//////////////////////////
// key=data_lpn, data=log_lpn
void   shashtbl_init(void);
void   shashtbl_insert(UINT32 const bank, UINT32 const key, UINT32 const data);
void   shashtbl_remove(UINT32 const bank, UINT32 const key);
void   shashtbl_update(UINT32 const bank, UINT32 const key, UINT32 const new_val);
UINT32 shashtbl_get(UINT32 const bank, UINT32 const key);
#endif // _SHASHTBL_H
