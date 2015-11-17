#define LINUX

#include <linux/module.h>  
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <asm/pgtable.h>
#include <asm/uaccess.h>

#define AUDIT_ENTRY_PML4 if(0)
#define AUDIT_ENTRY_PDPTE if(0)
#define AUDIT_ENTRY_PDE if(0)
#define AUDIT_ENTRY_PTE if(1)

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
	        	AUDIT_ENTRY_PML4 printk(KERN_ERR "\tPML4E_%d: %p \t Address:%p \t Control_bit:%p\n",index,entry,address,control_bit);
			AUDIT_ENTRY_PML4 printk(KERN_ERR "\t\t\t\t\t PA:%p \t\t VA:%p\n",real_address_pa,real_address_va);
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
	
	if(count != 0){	
		switch(level){
                	case 0:
                        	printk(KERN_ERR "[PML4E_BUSY]: %d\n",busy);
		        	printk(KERN_ERR "[PML4E_FREE]: %d\n",free);
				break;
                	case 1:
              			printk(KERN_ERR "\t\t\t (%d)[PDPTE_BUSY]: %d\n",index_parent,busy);
                        	printk(KERN_ERR "\t\t\t (%d)[PDPTE_FREE]: %d\n",index_parent,free);
                       		 break;  
			case 2:
                        	printk(KERN_ERR "\t\t\t\t\t\t (%d)[PDE_BUSY]: %d\n",index_parent,busy);
                        	printk(KERN_ERR "\t\t\t\t\t\t (%d)[PDE_FREE]: %d\n",index_parent,free);
                        	break;
                	case 3:
                       	 	printk(KERN_ERR "\t\t\t\t\t\t\t\t (%d)[PTE_BUSY]: %d\n",index_parent,busy);
                        	printk(KERN_ERR "\t\t\t\t\t\t\t\t (%d)[PTE_FREE]: %d\n",index_parent,free);
                       	 	break;
        	}
	}
	
}

int init_module(void){ 
	walk_table(0,0,current->mm->pgd);
	return 0;
}


void cleanup_module(void){}  

MODULE_LICENSE("GPL");

