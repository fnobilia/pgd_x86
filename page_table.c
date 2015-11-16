#define LINUX

#include <linux/module.h>  
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <asm/pgtable.h>
#include <asm/uaccess.h>

void print_entry(int level, int index,void *entry){
	void* control_bit;
        void* address;

	control_bit = (void *)((ulong) entry & 0x0000000000000fff);
        address = (void *)((ulong) entry & 0xfffffffffffff000);
	
	switch(level){
		case 0:
			printk(KERN_ERR "\t\tPML4E_%d: %p \t Address:%p \t Control_bit:%p\n",index,entry,address,control_bit);
			break;
		default: break;
	}
}

void walk_pml4(void){
	void** pml4;
        int index;
        int first_null;
	
	int busy;
	int free;
	
	pml4 = current->mm->pgd;
        printk(KERN_ERR "Address  PML4: %p\n",pml4);

        first_null = -1;
	busy = 0;
	free = 0;

        for(index=0; index<512; index++){
                if((pml4[index]==NULL)&&(first_null==-1)){
                        first_null = index;
                }
                else if((pml4[index]!=NULL)&&(first_null!=-1)){
                        //printk(KERN_ERR "\t\tPML4E from %d to %d are NULL\n",first_null,index-1);
                        first_null = -1;
                        print_entry(0,index,pml4[index]);
                }
                else if(pml4[index]!=NULL){
                        print_entry(0,index,pml4[index]);
                }
		
		if(pml4[index]!=NULL)	busy++;
		else free++;
        }
	
	printk(KERN_ERR "[PML4_BUSY]: %d\n",busy);
	printk(KERN_ERR "[PML4_FREE]: %d\n",free);
}

int init_module(void){ 
	walk_pml4();
	return 0;
}


void cleanup_module(void){}  

MODULE_LICENSE("GPL");

