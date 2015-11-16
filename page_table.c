#define LINUX

#include <linux/module.h>  
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <asm/pgtable.h>
#include <asm/uaccess.h>

void walk_table(int level, void **table);
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
	
	
	switch(level){
		case 0:
		//	 printk(KERN_ERR "\tPML4E_%d: %p \t Address:%p \t Control_bit:%p\n",index,entry,address,control_bit);
		//	 printk(KERN_ERR "\t\t\t\t\t PA:%p \t\t VA:%p\n",real_address_pa,real_address_va);
			break;
		case 1:
                //        printk(KERN_ERR "\t\tPDPTE_%d: %p \t Address:%p \t Control_bit:%p\n",index,entry,address,control_bit);
                //        printk(KERN_ERR "\t\t\t\t\t\t PA:%p \t\t VA:%p\n",real_address_pa,real_address_va);
                        break;
		case 2:
                //        printk(KERN_ERR "\t\t\tPDE_%d: %p \t Address:%p \t Control_bit:%p\n",index,entry,address,control_bit);
                //        printk(KERN_ERR "\t\t\t\t\t\t\t PA:%p \t\t VA:%p\n",real_address_pa,real_address_va);
                        break;
		case 3:
                //        printk(KERN_ERR "\t\t\t\tPTE_%d: %p \t Address:%p \t Control_bit:%p\n",index,entry,address,control_bit);
                //        printk(KERN_ERR "\t\t\t\t\t\t\t\t PA:%p \t\t VA:%p\n",real_address_pa,real_address_va);
                        break;
	}
	
	if(level == 3)	return;

	walk_table(level+1,real_address_va);
}

void walk_table(int level, void **table){
        int index;
        int first_null;
	int count;
	int busy;
	int free;
	
	switch(level){
                case 0:
			//printk(KERN_ERR "PML4: %p\n",table);
			break;
		case 1:
                        //printk(KERN_ERR "\tPDPT: %p\n",table);
                        break;
		case 2:
                        //printk(KERN_ERR "\t\tPD: %p\n",table);
                        break;
		case 3:
                        //printk(KERN_ERR "\t\t\tPT: %p\n",table);
                        break;
	}
	
	switch(level){
                case 0:
			count = 2;
                        break;
                case 1:
			count = 2;
                        break;
                case 2:
			count = 2;
                        break;
                case 3:
			count = 2;
                        break;
        }


        first_null = -1;
	busy = 0;
	free = 0;

        for(index=0; index<511; index++){
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
                        printk(KERN_ERR "[PML4_BUSY]: %d\n",busy);
		        printk(KERN_ERR "[PML4_FREE]: %d\n",free);
			break;
                case 1:
              		printk(KERN_ERR "\t[PDPT_BUSY]: %d\n",busy);
                        printk(KERN_ERR "\t[PDPT_FREE]: %d\n",free);
                        break;  
		case 2:
                        printk(KERN_ERR "\t\t[PD_BUSY]: %d\n",busy);
                        printk(KERN_ERR "\t\t[PD_FREE]: %d\n",free);
                        break;
                case 3:
                        printk(KERN_ERR "\t\t\t[PT_BUSY]: %d\n",busy);
                        printk(KERN_ERR "\t\t\t[PT_FREE]: %d\n",free);
                        break;
        }
	
}

int init_module(void){ 
	walk_table(0,current->mm->pgd);
	return 0;
}


void cleanup_module(void){}  

MODULE_LICENSE("GPL");

