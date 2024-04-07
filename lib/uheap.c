#include <inc/lib.h>
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
int32 user_array[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE]={0};
int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	//return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy
	int FF = sys_isUHeapPlacementStrategyFIRSTFIT();
	int32 limitt=sys_get_hard_limit();
	if(FF)
	{
		if(size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		{
			return alloc_block_FF(size);
		}
		else
		{
			size=ROUNDUP(size,PAGE_SIZE);
			uint32 numberofpages=size/PAGE_SIZE;

			uint32 count=0;
			uint32 sav ;
			uint32 str= 0;
			uint32 c=0;
			for(str = limitt + PAGE_SIZE; str < USER_HEAP_MAX; str += PAGE_SIZE)
			{
				int ind = (str - (limitt + PAGE_SIZE)) / PAGE_SIZE;
				if(user_array[ind] == 0)
				{
					if(count == 0)
					{
						 sav = str;
					}
					count+=1;
				}
				else
				{
					count=0;
					sav=-1;
				}
				if(count==numberofpages)
				{
					c=1;
					break;
				}
			}
			if(c==1)
			{
				int place =  (sav - (limitt + PAGE_SIZE))/ PAGE_SIZE ;
				for(int32 i=0;i<numberofpages;i++)
				{
					user_array[i+place] = numberofpages;
				}
				sys_allocate_user_mem(sav,size);
				return (void*)sav;

			}
			else
			{
				return NULL;
			}

		}
	}
	else
	{
		return NULL;
	}
	return NULL;

}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	int32 limitt=sys_get_hard_limit();
	if(virtual_address>=(void*)USER_HEAP_START&&virtual_address<(void*)limitt)
	{
		free_block(virtual_address);
	}
	else if((uint32)virtual_address>=limitt+PAGE_SIZE&&(uint32)virtual_address<USER_HEAP_MAX)
	{
		int32 place =  ((int32 ) virtual_address - (limitt + PAGE_SIZE))/ PAGE_SIZE ;
		uint32 no_of_pages=user_array[place];
		int32 size=user_array[place]*PAGE_SIZE;
		int32 i=0;
		sys_free_user_mem((int32 ) virtual_address ,size);
		for(int32 i=0;i<no_of_pages;i++)
		{
			user_array[place+i] = 0;
		}
	}
	else
		panic("..");
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
