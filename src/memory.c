#include "../include/function.h"

unsigned int memtest(unsigned int start, unsigned int end){
	char flag486 = 0;
	unsigned int eflag, cr0, i;

	/* 确认CPU是386还是486以上 */
	eflag = io_load_eflags();
	eflag |= EFLAGS_AC_BIT;	/* AC-bit = 1 */
	io_store_eflags(eflag);
	eflag = io_load_eflags();
	if((eflag & EFLAGS_AC_BIT) != 0) /* 如果是386，即使设定AC=1，AC的值还会自动回到0 */
		flag486 = 1;
	eflag &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflag);

	if(flag486 != 0){
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* 禁止缓存 */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if(flag486 != 0){
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* 允许缓存 */
		store_cr0(cr0);
	}

	return i;

}

void memman_init(MEMMAN *man){
	man->frees = 0;			/* 可用信息条数 */
	man->maxfrees = 0;		/* 用于观察可用状况：frees的最大值 */
	man->lostsize = 0;		/* 释放失败的内存大小总和 */
	man->losts = 0;			/* 释放失败次数 */
	return;
}

unsigned int memman_total(MEMMAN *man){
	unsigned int i;
	unsigned int total = 0;
	for(i = 0; i < man->frees; i++)
		total += man->free[i].size;
	return total;
}

unsigned int memman_alloc(MEMMAN *man, unsigned int size){
	unsigned int i;
	unsigned int addr;					/* 可分配地址 */
	for(i = 0; i < man->frees; i++){
		if(man->free[i].size >= size){	/* 找到足够大的内存 */
			addr = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size == 0){
				man->frees--;			/* 如果free[i]的大小变为0，就减少一条可用信息 */
				for(; i < man->frees; i++){
					man->free[i] = man->free[i+1];
				}
			}
			return addr;
		}
	}
	return 0;	/* 没有可分配空间 */
}

int memman_free(MEMMAN *man, unsigned int addr, unsigned int size){
	int i, j;
	/* 注：free[]中的条目按addr递增排序 */ 
	/* 确定条目插入位置 */
	for(i = 0; i < man->frees; i++)
		if(man->free[i].addr > addr)
			break;
	/* free[i-1].addr < addr < free[i].addr */
	if(i > 0){
		/* 前面有可用内存 */
		if(man->free[i-1].addr + man->free[i-1].size == addr){
			/* 可以和前面的可用内存归纳在一起 */
			man->free[i-1].size += size;
			if(i < man->frees){
				/* 后面有可用内存 */
				if(addr + size == man->free[i].addr){
					/* 可以和后面的可用内存归纳在一起 */
					man->free[i-1].size += man->free[i].size;
					/* man->free[i]删除 */
					man->frees--;
					for(; i < man->frees; i++){
						man->free[i] = man->free[i+1];
					}
				}
			}
			return 0;
		}
	}
	/* 不能与前面的内存归纳 */
	if(i < man->frees){
		/* 后面有可用内存 */
		if(addr + size == man->free[i].addr){
			/* 可以和后面的可用内存归纳在一起 */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}
	/* 既不能与前面的内存归纳又不能与前面的内存归纳 */
	if(man->frees < MEMMAN_FREES){
		/* free[i]之后的，向后移动，腾出一点可用空间 */
		for(j = man->frees; j > i; j--)
			man->free[j] = man->free[j-1];
		man->frees++;
		if(man->maxfrees < man->frees)
			man->maxfrees = man->frees;
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	/* 不能往后移动 */
	man->losts++;
	man->lostsize += size;
	return -1;	/* 失败 */
}

unsigned int memman_alloc_4k(MEMMAN *man, unsigned int size){
    unsigned int addr;
    size = (size + 0xfff) & 0xfffff000;     /* 以4KB为单位，将size向上舍入 */
    addr = memman_alloc(man, size);
    return addr;
}

int memman_free_4k(MEMMAN *man, unsigned int addr, unsigned int size){
	int status;
    size = (size + 0xfff) & 0xfffff000;     /* 以4KB为单位，将size向上舍入 */
    status = memman_free(man, addr, size);
    return status;
}
