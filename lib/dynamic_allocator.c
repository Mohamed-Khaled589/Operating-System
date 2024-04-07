/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list()
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &BLIST)
	{
		cprintf("(Address: %x, size: %d, isFree: %d)\n",blk, blk->size, blk->is_free) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
bool is_initialized=0;
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return ;
	is_initialized=1;
	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()

	//panic("initialize_dynamic_allocator is not implemented yet");



	// initialize a meta Data of first block should be initialize here
	struct BlockMetaData *blockMeta;
//	cprintf("%p\n",blockMeta);
	blockMeta = ((void*)daStart);
//	cprintf("%p\n",blockMeta);

	blockMeta->size = initSizeOfAllocatedSpace;
	blockMeta->is_free = 1;
	LIST_INSERT_HEAD(&BLIST,blockMeta);

}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{


	if (!is_initialized)
	{
	cprintf("=====================================================================\n");
	uint32 required_size = size + sizeOfMetaData();
	uint32 da_start = (uint32)sbrk(required_size);

	//get new break since it's page aligned! thus, the size can be more than the required one
	uint32 da_break = (uint32)sbrk(0);
	initialize_dynamic_allocator(da_start, da_break - da_start);
	}
	if(size == 0)
		return NULL;
	uint32 actualSize = size + sizeOfMetaData();
	struct BlockMetaData *currentBLock;
	struct BlockMetaData *currentBLock1;
	LIST_FOREACH(currentBLock,&BLIST)
	{
		if((currentBLock->size)>= actualSize && currentBLock->is_free)
		{
			currentBLock->is_free = 0;
			if((currentBLock->size -actualSize) >sizeOfMetaData())
			{
				struct BlockMetaData *newBlock;
				newBlock = (void *)currentBLock + actualSize;
				newBlock->size =  currentBLock->size - actualSize;
				newBlock->is_free = 1;
				currentBLock->size = actualSize;
				LIST_INSERT_AFTER(&BLIST,currentBLock, newBlock);
			}
			return (void *)currentBLock + sizeOfMetaData();
		}
	}
	// SBRK ==============
	void *sbrkAddr = sbrk(size);
	if(sbrkAddr==(void *)-1)
	{
		return NULL;
	}
	struct BlockMetaData *sbrkBlock=(struct BlockMetaData *)sbrkAddr;
	sbrkBlock->is_free=1;
	sbrkBlock->size=sbrk(0)-sbrkAddr;
	LIST_INSERT_TAIL(&BLIST,sbrkBlock);
		sbrkBlock->is_free = 0;
		if((sbrkBlock->size -actualSize) >sizeOfMetaData())
		{
			struct BlockMetaData *newBlock;
			newBlock = (void *)sbrkBlock + actualSize;
			newBlock->size =  sbrkBlock->size - actualSize;
			newBlock->is_free = 1;
			sbrkBlock->size = actualSize;
			LIST_INSERT_AFTER(&BLIST,sbrkBlock, newBlock);

		}
		return (void *)sbrkBlock + sizeOfMetaData();
//	}

}

void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	//panic("alloc_block_BF is not implemented yet");


	if(size == 0)
		return NULL;
	uint32 actualSize = size + sizeOfMetaData();
	struct BlockMetaData *currentBLock;
	uint32 min_dif=314163345;
	struct BlockMetaData *bestblock;

	LIST_FOREACH(currentBLock,&BLIST)
	{

		// loop to find best block
		if(currentBLock->is_free&&currentBLock->size >= actualSize)
		{


			 if((currentBLock->size-actualSize)<=min_dif)
			{

				//check for minimum difference to know who id the best block
				min_dif=currentBLock->size-actualSize;
				bestblock=currentBLock;
			}

		}

	}

	if(min_dif!=314163345)
	{

		if(min_dif<sizeOfMetaData())
		{

			//if min_dif==0 we don't want to split
			bestblock->is_free=0;
			bestblock->size=actualSize;

			return (void *)bestblock + sizeOfMetaData();
		}
		else
		{

			struct BlockMetaData *newBlock;
			newBlock = (void *)bestblock + actualSize;
			newBlock->size =  bestblock->size - actualSize;
			newBlock->is_free = 1;
			bestblock->size = actualSize;
			bestblock->is_free=0;
			LIST_INSERT_AFTER(&BLIST,bestblock, newBlock);

			return (void *)bestblock + sizeOfMetaData();
		}
	}

	else
	{

		// if there not any space then sbrk my brother
		struct BlockMetaData *sbrkBlock = sbrk(size);
		// if couldn't return NULL
		if(sbrkBlock == (void *) -1){
			return NULL;
		}
		sbrkBlock->is_free = 0;
		sbrkBlock->size = size;
		return sbrkBlock + sizeOfMetaData();
	}





	return NULL;

}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{

		//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
	//	panic("free_block is not implemented yet");
		if(va !=NULL)
		{

			struct BlockMetaData *BlkMetaDataptr = ((struct BlockMetaData *)va-1),*NextBlock=BlkMetaDataptr->prev_next_info.le_next;
			struct BlockMetaData *PrevBlock=BlkMetaDataptr->prev_next_info.le_prev;
			if(NextBlock==NULL)
			{
				if(PrevBlock->is_free==1)
				{
					BlkMetaDataptr->is_free=0;
					(PrevBlock->size)=(BlkMetaDataptr->size)+(PrevBlock->size);
					BlkMetaDataptr->size=0;
					LIST_REMOVE(&BLIST,BlkMetaDataptr);
					BlkMetaDataptr=NULL;

				}
				else
				{
					BlkMetaDataptr->is_free=1;
				}
			}
			else if(PrevBlock==NULL)
			{
				if(NextBlock->is_free==1)
				{

					BlkMetaDataptr->is_free=1;
					BlkMetaDataptr->size=(BlkMetaDataptr->size)+(NextBlock->size);
					NextBlock->is_free=0;
					NextBlock->size=0;
					LIST_REMOVE(&BLIST,NextBlock);
					NextBlock=NULL;
				}
				else
				{
					BlkMetaDataptr->is_free=1;
				}

			}

			else
			{
				if((NextBlock->is_free==1) &&(PrevBlock->is_free==1))
				{
					PrevBlock->size=(NextBlock->size)+(BlkMetaDataptr->size)+(PrevBlock->size);
					NextBlock->size=0;
					NextBlock->is_free=0;
					BlkMetaDataptr->size=0;
					BlkMetaDataptr->is_free=0;
					LIST_REMOVE(&BLIST,NextBlock);
					LIST_REMOVE(&BLIST,BlkMetaDataptr);
					BlkMetaDataptr=NULL;
					NextBlock=NULL;

				}
				else if((NextBlock->is_free==1) &&(PrevBlock->is_free==0))
				{
					BlkMetaDataptr->size=(BlkMetaDataptr->size)+(NextBlock->size);
					BlkMetaDataptr->is_free=1;
					NextBlock->size=0;
					NextBlock->is_free=0;
					LIST_REMOVE(&BLIST,NextBlock);
					NextBlock=NULL;
				}
				else if((NextBlock->is_free==0) && (PrevBlock->is_free==1))
				{
					PrevBlock->size=(PrevBlock->size)+(BlkMetaDataptr->size);
					BlkMetaDataptr->is_free=0;
					BlkMetaDataptr->size=0;
					LIST_REMOVE(&BLIST,BlkMetaDataptr);
					BlkMetaDataptr=NULL;
				}
				else if((NextBlock->is_free==0) && (PrevBlock->is_free==0))
				{
					BlkMetaDataptr->is_free=1;

				}
			}
		}
	}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void* realloc_block_FF(void* va, uint32 new_size)
{
//	initialize_dynamic_allocator(KERNEL_HEAP_START,(3*1024*1204));

	//,
	//panic("realloc_block_FF is not implemented yet");
	if (va==NULL)
	{

			return alloc_block_FF(new_size);

	}
	else if(new_size==0)
	{

		free_block(va);
		return NULL;
	}

	struct BlockMetaData *ptr= ((struct BlockMetaData *)va - 1) ;
	uint32 actualSize1 = new_size + sizeOfMetaData();
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	struct BlockMetaData *NextBlkMetaData = (struct BlockMetaData *)curBlkMetaData->prev_next_info.le_next;
	LIST_FOREACH(ptr,&(BLIST))
	{
	if (actualSize1>(curBlkMetaData->size))
	 {

			if(is_free_block(NextBlkMetaData) && (get_block_size(NextBlkMetaData))>= actualSize1-curBlkMetaData->size )
			  {

				//initialize new block
				struct BlockMetaData *newBlock ;
				newBlock=(void * )curBlkMetaData+actualSize1;
				//update data of new block
				newBlock->size = (uint32) NextBlkMetaData->size - (uint32 )(actualSize1 - curBlkMetaData->size )  ;
				newBlock->is_free = 1;
				//update current size by new size
				curBlkMetaData->size = (uint32)actualSize1;
				// add new block to linked list and delete old next block
				LIST_INSERT_AFTER(&BLIST,curBlkMetaData,newBlock);
				NextBlkMetaData->is_free=0;
				NextBlkMetaData->size=0;
				NextBlkMetaData->prev_next_info.le_next=NULL;
				NextBlkMetaData->prev_next_info.le_prev=NULL;


				return (void *)curBlkMetaData + sizeOfMetaData();
			  }

			else
			 {
			// use alloc_block_FF to search for new block
			struct BlockMetaData *test_return = (struct BlockMetaData *)alloc_block_FF((uint32)new_size);
				if(test_return!=NULL)
				{
					// if you find suitable block free current block
					// add " & " if there is any error to try to solve it
					struct BlockMetaData *save_return = (struct BlockMetaData *)memcpy(test_return,curBlkMetaData,curBlkMetaData->size-sizeOfMetaData());
					free_block(&curBlkMetaData);
					return (void *)save_return;
				}
			  }
			// if there not any space then sbrk
			// if couldn't return NULL

			struct BlockMetaData *sbrkBlock = sbrk((uint32)new_size);
			if(sbrkBlock == (void *) -1){
				return NULL;
			}
			sbrkBlock->is_free = 0;
			sbrkBlock->size = (uint32 )new_size+sizeOfMetaData();
			return sbrkBlock +sizeOfMetaData();
	 }

	else if(actualSize1<=curBlkMetaData->size)
	{

		//initialize new block & we put +1 to point to the block
		struct BlockMetaData *newBlock =((void *) curBlkMetaData + actualSize1);
		//update data of new block
		newBlock->size = curBlkMetaData->size -(uint32)actualSize1   ;
		newBlock->is_free = 1;
		//update current size by new size
		curBlkMetaData->size = (uint32)actualSize1;
		// add new block to linked list and delete old next block
		LIST_INSERT_AFTER(&BLIST,curBlkMetaData,newBlock);
		if(NextBlkMetaData->is_free==1)
					{

						free_block(NextBlkMetaData);

					}
		return (void *)curBlkMetaData + sizeOfMetaData() ;


	}
	}
	return NULL;

}

