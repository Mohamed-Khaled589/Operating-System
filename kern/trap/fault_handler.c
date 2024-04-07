/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"
#include <inc/queue.h>

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{

	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
		int iWS =curenv->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(curenv);
#endif

//		cprintf("fault va = %x\n",fault_va);
		if(isPageReplacmentAlgorithmFIFO())
		{
			//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
			// Write your code here, remove the panic and write your code
			//panic("page_fault_handler() FIFO Replacement is not implemented yet...!!");
			if(wsSize <(curenv->page_WS_max_size))
			{
					//cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
					//TODO: [PROJECT'23.MS25 - #15] [3] PAGE FAULT HANDLER - Placement
					// Write your code here, remove the panic and write your code
					//panic("page_fault_handler().PLACEMENT is not implemented yet...!!");
					//refer to the project presentation and documentation for details
					struct FrameInfo *ptr_frame_info;
					int ret=allocate_frame(&ptr_frame_info);
					if (ret !=0)
					{
						panic("no free frame ");
					}
					map_frame(curenv->env_page_directory ,ptr_frame_info,fault_va,PERM_WRITEABLE|PERM_USER);
					int ret2=pf_read_env_page(curenv,(void *)fault_va);
					struct WorkingSetElement* NewWorkingSet1=env_page_ws_list_create_element(curenv,fault_va);
					LIST_INSERT_TAIL(&(curenv->page_WS_list),NewWorkingSet1);
//					curenv->page_last_WS_element=curenv->page_last_WS_element->prev_next_info.le_next;
					if(ret2==E_PAGE_NOT_EXIST_IN_PF)
					{
						int32 b1=0,b2=0;
						if(fault_va>=USER_HEAP_START && fault_va < USER_HEAP_MAX)
						{
							b1=1;

						}
						if(fault_va >= USTACKBOTTOM && fault_va < USTACKTOP)
						{
							b2=1;

						}
						if(b1==0 && b2==0)
						{
							sched_kill_env(curenv->env_id);
						}
					if(wsSize==curenv->page_WS_max_size)
					{
						curenv->page_last_WS_element=LIST_FIRST(&curenv->page_WS_list);
					}
				  }

			}
			else
			{

//					env_page_ws_print(curenv);
//					cprintf("last element =%x\n",curenv->page_last_WS_element);
					uint32 page_permission=pt_get_page_permissions(curenv->env_page_directory,curenv->page_last_WS_element->virtual_address);
					if(page_permission & PERM_MODIFIED)
					{
					uint32* ptr_page_table=NULL;
					struct FrameInfo *ptr_frame_info = get_frame_info(curenv->env_page_directory,curenv->page_last_WS_element->virtual_address,&ptr_page_table);
					pf_update_env_page(curenv,curenv->page_last_WS_element->virtual_address , ptr_frame_info);
					}
					struct WorkingSetElement* NewWorkingSet=env_page_ws_list_create_element(curenv,fault_va);
					struct FrameInfo *ptr_frame_info;
					int ret=allocate_frame(&ptr_frame_info);
					map_frame(curenv->env_page_directory ,ptr_frame_info,fault_va,PERM_WRITEABLE|PERM_USER);
					LIST_INSERT_AFTER(&(curenv->page_WS_list),curenv->page_last_WS_element,NewWorkingSet);
					unmap_frame(curenv->env_page_directory,curenv->page_last_WS_element->virtual_address);
					env_page_ws_invalidate(curenv,(curenv->page_last_WS_element->virtual_address));
					int ret2=pf_read_env_page(curenv,(void*) fault_va);
					if(ret2==E_PAGE_NOT_EXIST_IN_PF)
					{
						int32 b1=0,b2=0;
						if(fault_va>=USER_HEAP_START && fault_va < USER_HEAP_MAX)
						{
							b1=1;

						}
						if(fault_va >= USTACKBOTTOM && fault_va < USTACKTOP)
						{
							b2=1;

						}
						if(b1==0 && b2==0)
						{
							sched_kill_env(curenv->env_id);
						}
					}

					if(curenv->page_last_WS_element->prev_next_info.le_next==NULL)
					{
						curenv->page_last_WS_element=LIST_FIRST(&curenv->page_WS_list);
//						cprintf("last element 1 =%x\n",curenv->page_last_WS_element);

					}
					else
					{
						curenv->page_last_WS_element=NewWorkingSet->prev_next_info.le_next;
//						cprintf("last element 2 =%x\n",curenv->page_last_WS_element);
					}
			}

	}

	else
	{
//			cprintf("fault va = %x\n",fault_va);
//			cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
			//refer to the project presentation and documentation for details
			//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
			// Write your code here, remove the panic and write your code
			//panic("page_fault_handler() LRU Replacement is not implemented yet...!!");
			//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement
//			curenv->ActiveListSize=(curenv->page_WS_max_size)-(curenv->SecondListSize);
//			cprintf("Max Size For Process = %d\n",(curenv->page_WS_max_size));
//			cprintf("Max Active list Size = %d\n",curenv->ActiveListSize);
//			cprintf("Active list Size = %d\n",LIST_SIZE(&curenv->ActiveList));
//			cprintf("Max Second list Size = %d\n",curenv->SecondListSize);
//			cprintf("Second list Size = %d\n",LIST_SIZE(&curenv->SecondList));
			if((LIST_SIZE(&curenv->ActiveList)+LIST_SIZE(&curenv->SecondList))<curenv->page_WS_max_size)
			{
				//PLACEMENT
				if(LIST_SIZE(&curenv->ActiveList)<(curenv->ActiveListSize))
				{
					struct FrameInfo *ptr_frame_info;
					int ret=allocate_frame(&ptr_frame_info);
					map_frame(curenv->env_page_directory ,ptr_frame_info,fault_va,PERM_WRITEABLE|PERM_USER);
					int ret2=pf_read_env_page(curenv,(void *)fault_va);
					struct WorkingSetElement* NewWorkingSet1=env_page_ws_list_create_element(curenv,fault_va);
					LIST_INSERT_HEAD(&(curenv->ActiveList),NewWorkingSet1);
				}
				else
				{
					struct WorkingSetElement* var;
					struct WorkingSetElement* tail_of_second=LIST_LAST(&(curenv->SecondList));
					struct WorkingSetElement* tail_of_active=LIST_LAST(&(curenv->ActiveList));
					int flag=0;
					LIST_FOREACH(var,&curenv->SecondList)
					{

						if(ROUNDDOWN(var->virtual_address,PAGE_SIZE)==ROUNDDOWN(fault_va,PAGE_SIZE))
						{
							struct WorkingSetElement* NewWorkingSet1=env_page_ws_list_create_element(curenv,fault_va);
							pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);
							pt_set_page_permissions(curenv->env_page_directory,tail_of_active->virtual_address,0,PERM_PRESENT);
							flag=1;
							LIST_REMOVE(&(curenv->SecondList),var);
							LIST_REMOVE(&(curenv->ActiveList),tail_of_active);
							LIST_INSERT_HEAD(&(curenv->SecondList),tail_of_active);
							LIST_INSERT_HEAD(&(curenv->ActiveList),var);
							break;
						}
					}
					if(flag==0)
					{
						struct FrameInfo *ptr_frame_info;
						int ret=allocate_frame(&ptr_frame_info);
			//			cprintf("+++++++++++++++++++++++++++++++++++1\n");
						map_frame(curenv->env_page_directory ,ptr_frame_info,fault_va,PERM_WRITEABLE|PERM_USER);
						int ret2=pf_read_env_page(curenv,(void *)fault_va);
						struct WorkingSetElement* NewWorkingSet1=env_page_ws_list_create_element(curenv,fault_va);
						struct WorkingSetElement* tail=LIST_LAST(&(curenv->ActiveList));
						//if(tail!=NULL)
						LIST_REMOVE(&(curenv->ActiveList),tail);
						LIST_INSERT_HEAD(&(curenv->SecondList),tail);
						LIST_INSERT_HEAD(&(curenv->ActiveList),NewWorkingSet1);
						pt_set_page_permissions(curenv->env_page_directory,tail->virtual_address,0,PERM_PRESENT);
					}
				}
//				env_page_ws_print(curenv);

			}
			else
			{
				//Replacement
				struct WorkingSetElement* var;
				struct WorkingSetElement* tail_of_second=LIST_LAST(&(curenv->SecondList));
				struct WorkingSetElement* tail_of_active=LIST_LAST(&(curenv->ActiveList));
				int flag=0;
				LIST_FOREACH(var,&curenv->SecondList)
				{

					if(ROUNDDOWN(var->virtual_address,PAGE_SIZE)==ROUNDDOWN(fault_va,PAGE_SIZE))
					{
						struct WorkingSetElement* NewWorkingSet1=env_page_ws_list_create_element(curenv,fault_va);
						flag=1;
						LIST_REMOVE(&(curenv->ActiveList),tail_of_active);
						LIST_REMOVE(&(curenv->SecondList),var);
						pt_set_page_permissions(curenv->env_page_directory,var->virtual_address,PERM_PRESENT,0);
						LIST_INSERT_HEAD(&(curenv->ActiveList),var);
						pt_set_page_permissions(curenv->env_page_directory,tail_of_active->virtual_address,0,PERM_PRESENT);
						LIST_INSERT_HEAD(&(curenv->SecondList),tail_of_active);
						break;
					}
				}
				if(flag==0)
				{
					struct WorkingSetElement* tail_of_second=LIST_LAST(&(curenv->SecondList));
					struct WorkingSetElement* tail_of_active=LIST_LAST(&(curenv->ActiveList));
					struct WorkingSetElement* NewWorkingSet1=env_page_ws_list_create_element(curenv,fault_va);
					uint32 page_permission=pt_get_page_permissions(curenv->env_page_directory,tail_of_second->virtual_address);
					uint32* ptr_page_table=NULL;
					struct FrameInfo *ptr_frame_info = get_frame_info(curenv->env_page_directory,tail_of_second->virtual_address,&ptr_page_table);
					if(page_permission & PERM_MODIFIED)
					{
						pf_update_env_page(curenv,tail_of_second->virtual_address , ptr_frame_info);
					}
					LIST_REMOVE(&(curenv->SecondList),tail_of_second);
					unmap_frame(curenv->env_page_directory,tail_of_second->virtual_address);
					LIST_REMOVE(&(curenv->ActiveList),tail_of_active);
					pt_set_page_permissions(curenv->env_page_directory,tail_of_active->virtual_address,0,PERM_PRESENT);
					LIST_INSERT_HEAD(&(curenv->SecondList),tail_of_active);
					int ret=allocate_frame(&ptr_frame_info);
					map_frame(curenv->env_page_directory ,ptr_frame_info,fault_va,PERM_WRITEABLE|PERM_USER);
					int ret2=pf_read_env_page(curenv,(void *)fault_va);
					LIST_INSERT_HEAD(&(curenv->ActiveList),NewWorkingSet1);
//					env_page_ws_print(curenv);
				}
//				env_page_ws_print(curenv);
			}
		}

}
void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}


