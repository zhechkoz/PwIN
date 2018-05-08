#include <linux/module.h>
#include <linux/kernel.h>

static inline u64 getcr4(void) {
    u64 cr4;
    
    asm volatile (
        "mov %%cr4, %%rax\n\t"
        "mov %%rax, %0\n\t"
        :"=m"(cr4)
        :: "%rax"
    );
    
    return cr4;
}

static void setfsgsbase(void* info) {
    u64 cr4 = getcr4(); 
    printk(KERN_INFO "FSGSBASE original: %llu (%u).\n", cr4, (unsigned char)((cr4 >> 16) & 1));
	
	cr4 |= (1 << 16);

	asm volatile (
        "movq %0, %%cr4\n"
        :
        :"r"(cr4)
    );
    
    printk(KERN_INFO "FSGSBASE changed: %llu (%u).\n", cr4, (unsigned char)((cr4 >> 16) & 1));
}

int init_module(void) {
	on_each_cpu(setfsgsbase, NULL, 0);
	return 0;
}

void cleanup_module(void) {
}

