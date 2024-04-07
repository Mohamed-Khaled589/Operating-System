#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
int32 kheap_array[(KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE];

int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

	//Comment the following line(s) before start coding...
	//panic("not implemented yet");
		start=daStart;
		brk=daStart+initSizeToAllocate;
		limit=daLimit;

	for (uint32 va = start; va < brk; va += PAGE_SIZE)
	{
		struct FrameInfo* ptr_frame;
		int ref=allocate_frame(&ptr_frame);
		if(ref!=0)
		{
			return E_NO_MEM;
		}
		else
		{

			int x=map_frame(ptr_page_directory,ptr_frame,va,PERM_USER|PERM_WRITEABLE);
			ptr_frame->va=va;
			if(x!=0)
				return E_NO_MEM;

		}
	}
		initialize_dynamic_allocator(start,initSizeToAllocate);
		return 0;

}

void* sbrk(int increment)
{
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */


	uint32 new_increment=ROUNDUP(increment,PAGE_SIZE);
	uint32 num_of_page=new_increment/PAGE_SIZE;


	if(increment>0)
	{
		if(brk+new_increment>limit)
			panic("bla bla bla !!!!");
		else
		{

			for (uint32 va = brk; va < (brk+new_increment); va += PAGE_SIZE)
			{

				struct FrameInfo* ptr_frame;
				int ref=allocate_frame(&ptr_frame);
				if(ref!=0)
				{
					panic("bla bla bla !!!!");
				}
				else
				{

						int x=map_frame(ptr_page_directory,ptr_frame,va,PERM_WRITEABLE);
						if(x!=0)
							panic("bla bla bla !!!!");

						ptr_frame->va=va;


				}
			}

			brk=brk+new_increment;

		}
		return (void*)(brk - new_increment);
	}
	else if(increment<0)
	{
		cprintf("from sbrk\n");
		uint32 new_increment=ROUNDDOWN(increment,PAGE_SIZE);

		if(brk-new_increment<start)
			panic("bla bla bla !!!!");
		struct FrameInfo* ptr_frame;
		uint32 *ptr_page_table;
		for (uint32 va = brk; va > brk-new_increment; va -= PAGE_SIZE)
		{

		get_frame_info(ptr_page_directory,va,&ptr_page_table);
		ptr_frame=(struct FrameInfo *)ptr_page_table;
		unmap_frame(ptr_page_directory,va);
		free_frame(ptr_frame);

		}

			brk=brk-new_increment;
			return (void*)brk;


	}
	else if(increment==0)
	{

		return (void*)brk;

	}
	//MS2: COMMENT THIS LINE BEFORE START CODING====
	panic("bla bla bla !!!!");
	//panic("not implemented yet");
}
void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	if(size<=DYN_ALLOC_MAX_BLOCK_SIZE)
	{

		return alloc_block_FF(size);

	}
	else
	{
		size=ROUNDUP(size,PAGE_SIZE);
		uint32 bla;
		struct FrameInfo *ptr_frame;
		uint32 numberofpages=size/PAGE_SIZE;
		uint32 count=0;
		uint32 sav ;
		uint32 str=limit+PAGE_SIZE;
		uint32 c=0;
		for(uint32 str=limit+PAGE_SIZE;str<KERNEL_HEAP_MAX;str+=PAGE_SIZE)
		{
			uint32 *ptr_page_table=NULL;
			struct FrameInfo *fr;
			fr=get_frame_info(ptr_page_directory,str,&ptr_page_table);

			if(fr == 0)
			{
				if(count ==0)
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


			for(uint32 var=sav;var<sav+numberofpages*PAGE_SIZE;var+=PAGE_SIZE)
				{
					struct FrameInfo* ptr_frame;
					int ref=allocate_frame(&ptr_frame);
					if(ref!=0)
					{

						return NULL;
					}
					else
					{

							int x=map_frame(ptr_page_directory,ptr_frame,var,PERM_WRITEABLE);
							if(x!=0)
							{
								return NULL;
							}
							ptr_frame->va=var;

					}
				}
			int index =  (sav - KERNEL_HEAP_START)/ PAGE_SIZE ;
			kheap_array[index] = numberofpages;

			return (void*)sav;
		}
		else
			return NULL;
	}
	return NULL;
}



void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
//




		if((uint32)virtual_address>=start&&(uint32)virtual_address<limit)
		{
			free_block(virtual_address);
		}
		else
		{
		int place =  ((int32 ) virtual_address - KERNEL_HEAP_START)/ PAGE_SIZE ;

		for(int i = 0 ; i < kheap_array[place] ; i++)
		{
			struct FrameInfo* frame_info = NULL;
			uint32* ptr_page_table = NULL;
			uint32 va = ((int32)virtual_address + (i * PAGE_SIZE));

			get_page_table(ptr_page_directory,(va+ (i * PAGE_SIZE)), &ptr_page_table);
			frame_info = get_frame_info(ptr_page_directory,(uint32) (va+ (i * PAGE_SIZE)), &ptr_page_table);
			unmap_frame(ptr_page_directory,  va);

		}
		kheap_array[place] = 0;
		}

}
unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	//change this "return" according to your answer
		uint32 fram_num=(physical_address>>12);
		uint32 offset=(physical_address<<20);
		offset=offset>>20;
		uint32 ret=(fram_num*PAGE_SIZE)+offset;
		struct FrameInfo* frame_info =NULL;
		frame_info=to_frame_info(physical_address);
		if(frame_info->va!=(uint32)NULL)
		{
			return (frame_info->va)+offset;

		}
		return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");
	uint32* ptr_page_table = NULL;
	get_page_table(ptr_page_directory,virtual_address, &ptr_page_table);
	unsigned int ret = (ptr_page_table[PTX(virtual_address)] >> 12) * PAGE_SIZE;
	uint32 offset=(virtual_address<<20);
	offset=offset>>20;
	return (ret+offset);
	//change this "return" according to your answer
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
