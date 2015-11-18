#define LINUX

#include <linux/module.h>  
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <asm/pgtable.h>
#include <asm/uaccess.h>

#define AUDIT_ENTRY_PML4E if(0)
#define AUDIT_ENTRY_PDPTE if(0)
#define AUDIT_ENTRY_PDE if(0)
#define AUDIT_ENTRY_PTE if(1)

#define AUDIT_RESULT_PML4E if(1)
#define AUDIT_RESULT_PDPTE if(1)
#define AUDIT_RESULT_PDE if(1)
#define AUDIT_RESULT_PTE if(0)

#define AUDIT_ADDRESS if(0)

void walk_table(int level, int index_parent, void **table);
void manage_entry(int level, int index,void *entry);

void manage_entry(int level, int index,void *entry){
	void* control_bit;
        void* address;
        void* real_address_pa;
        void* real_address_va;

	control_bit = (void *)((ulong) entry & 0x0000000000000fff);
        address = (void *)((ulong) entry & 0xfffffffffffff000);
        real_address_pa = address;
        real_address_va = __va(real_address_pa);
	
	if(!((ulong)control_bit ^ 0x0000000000000061))	return;
	
	switch(level){
		case 0:
	        	AUDIT_ENTRY_PML4E printk(KERN_ERR "\tPML4E_%d: %p \t Address:%p \t Control_bit:%p\n",index,entry,address,control_bit);
			AUDIT_ENTRY_PML4E printk(KERN_ERR "\t\t\t\t\t PA:%p \t\t VA:%p\n",real_address_pa,real_address_va);
			break;
		case 1:
	        	AUDIT_ENTRY_PDPTE printk(KERN_ERR "\t\tPDPTE_%d: %p \t Address:%p \t Control_bit:%p\n",index,entry,address,control_bit);
                    	AUDIT_ENTRY_PDPTE printk(KERN_ERR "\t\t\t\t\t\t PA:%p \t\t VA:%p\n",real_address_pa,real_address_va);
                        break;
		case 2:
	        	AUDIT_ENTRY_PDE printk(KERN_ERR "\t\t\tPDE_%d: %p \t Address:%p \t Control_bit:%p\n",index,entry,address,control_bit);
                      	AUDIT_ENTRY_PDE printk(KERN_ERR "\t\t\t\t\t\t\t PA:%p \t\t VA:%p\n",real_address_pa,real_address_va);
                        break;
		case 3:
                	AUDIT_ENTRY_PTE printk(KERN_ERR "\t\t\t\tPTE_%d: %p \t Address:%p \t Control_bit:%p\n",index,entry,address,control_bit);
                	AUDIT_ENTRY_PTE printk(KERN_ERR "\t\t\t\t\t\t\t\t PA:%p \t\t VA:%p\n",real_address_pa,real_address_va);
                        break;
	}
	
	if(level == 3)	return;

	walk_table(level+1,index,real_address_va);
}

void walk_table(int level, int index_parent, void **table){
        int index;
        int first_null;
	int busy;
	int free;
	int count;
	
	AUDIT_ADDRESS	
	switch(level){
                case 0:
			printk(KERN_ERR "PML4: %p\n",table);
			break;
		case 1:
                        printk(KERN_ERR "\tPDPT: %p\n",table);
                        break;
		case 2:
                        printk(KERN_ERR "\t\tPD: %p\n",table);
                        break;
		case 3:
                        printk(KERN_ERR "\t\t\tPT: %p\n",table);
                        break;
	}
	
	switch(level){
                case 0:
                        count = 512;
                        break;
                case 1:
                        count = 512;
                        break;
                case 2:
                        count = 512;
                        break;
                case 3:
                        count = 0;
                        break;
        }
	

        first_null = -1;
	busy = 0;
	free = 0;

        for(index=0; index<count; index++){
                if((table[index]==NULL)&&(first_null==-1)){
                        first_null = index;
                }
                else if((table[index]!=NULL)&&(first_null!=-1)){
                        first_null = -1;
                        manage_entry(level,index,table[index]);
			
			//if(level<2)	break;
                }
                else if(table[index]!=NULL){
                        manage_entry(level,index,table[index]);
			
			//if(level<2)	break;
                }
		
		if(table[index]!=NULL)	busy++;
		else free++;
        }
	
	switch(level){
		case 0:
			AUDIT_RESULT_PML4E printk(KERN_ERR "[PML4E_BUSY]: %d\n",busy);
			AUDIT_RESULT_PML4E printk(KERN_ERR "[PML4E_FREE]: %d\n",free);
			break;
		case 1:
			AUDIT_RESULT_PDPTE printk(KERN_ERR "\t\t\t (%d)[PDPTE_BUSY]: %d\n",index_parent,busy);
			AUDIT_RESULT_PDPTE printk(KERN_ERR "\t\t\t (%d)[PDPTE_FREE]: %d\n",index_parent,free);
			break;  
		case 2:
			AUDIT_RESULT_PDE printk(KERN_ERR "\t\t\t\t\t\t (%d)[PDE_BUSY]: %d\n",index_parent,busy);
			AUDIT_RESULT_PDE printk(KERN_ERR "\t\t\t\t\t\t (%d)[PDE_FREE]: %d\n",index_parent,free);
			break;
		case 3:
			AUDIT_RESULT_PTE printk(KERN_ERR "\t\t\t\t\t\t\t\t (%d)[PTE_BUSY]: %d\n",index_parent,busy);
			AUDIT_RESULT_PTE printk(KERN_ERR "\t\t\t\t\t\t\t\t (%d)[PTE_FREE]: %d\n",index_parent,free);
			break;
	}
	
}

/*pgd_t *pgd;
pud_t *pud;
pmd_t *pmd;
pte_t *pte;*/

void create_new_pgd(void){
	void* pgd_addr  = (void *)__get_free_pages(GFP_KERNEL, 0);
	memcpy((void *)pgd_addr, (void *)(current->mm->pgd), 4096);

	void** pgd_entry = (void **)pgd_addr;
	void** original_pdpt_entry;
	
	int i;
	void* pml4_entry;
	void* address_pdpt;
	void** new_pdpt_entry;			
	void* address_pd;
	void* temp;
	for (i=0; i<PTRS_PER_PGD; i++){
		//Current entry		
		pml4_entry = pgd_entry[i];

		if(pml4_entry != NULL){
			//New page PDPT
	                address_pdpt = (void *)__get_free_pages(GFP_KERNEL, 0);
        	        memset(address_pdpt,0,4096);

			//Control bits
			pml4_entry = (void *)((ulong) pml4_entry & 0x0000000000000fff);	
			//printk(KERN_ERR "PML4_entry[control bit]: %p \t pgd_entry[%d]: %p \n",pml4_entry,i,pgd_entry[i]);
			
			//Final value of PML4E
			address_pdpt = (void *)__pa(address_pdpt);
                        pml4_entry = (void *)((ulong)address_pdpt | (ulong)pml4_entry);
			//printk(KERN_ERR "PML4_entry: %p \t pgd_entry[%d]: %p \n",pml4_entry,i,pgd_entry[i]);
			
			//Pointer to Orinal PDPT
			temp = (void *)((ulong) pgd_entry[i] & 0xfffffffffffff000);
                        temp = (void *)(__va(temp));
			original_pdpt_entry = (void **)temp;
			//printk(KERN_ERR "Original_PDPT_entry: %p \n",original_pdpt_entry);
			
			//Pointer to new PDPT			
			temp = (void *)((ulong) pml4_entry & 0xfffffffffffff000);
                        temp = (void *)(__va(temp));
                        new_pdpt_entry = (void **)temp;
                        //printk(KERN_ERR "Original_PDPT_entry: %p \t New_PDPT_entry: %p \n",original_pdpt_entry,new_pdpt_entry);
			
			int j;
			void* address_pd;
			void* pdpt_entry;
			for(j=0; j<PTRS_PER_PUD; j++){
				if(original_pdpt_entry[j] != NULL){
                        		//New page PD
                        		address_pd = (void *)__get_free_pages(GFP_KERNEL, 0);
                        		memset(address_pd,0,4096);
					
					//Control bits
                        		pdpt_entry = (void *)((ulong) original_pdpt_entry[j] & 0x0000000000000fff);
					
					//Final value of PDPTE
		                        address_pd = (void *)__pa(address_pd);
                		        pdpt_entry = (void *)((ulong)address_pd | (ulong)pdpt_entry);
					
					//Update new PDPTE					
					new_pdpt_entry[j] = pdpt_entry;
				}			
			}
			
			//Update new PML4E
			pgd_entry[i] = pml4_entry;						
		}		
	}
	
	walk_table(0,0,pgd_addr);	
}

int init_module(void){ 
	walk_table(0,0,current->mm->pgd);
	create_new_pgd();
	return 0;
}


void cleanup_module(void){}  

MODULE_LICENSE("GPL");

