#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/*
 * Ptmalloc2会验证当前释放的chunk是否和上次释放的chunk是同一个，如果是，则说明发生了double free，
 * 这个逻辑很直接，也很容易绕过，演示如下：
 */

int main()
{
	setbuf(stdout, NULL);

	printf("本文件使用fastbins演示了一个简单的double-free工具.\n");

	printf("首先填满8个tcache\n");
	void *ptrs[8];
	for (int i=0; i<8; i++) {
		ptrs[i] = malloc(8);
	}
	for (int i=0; i<7; i++) {
		free(ptrs[i]);
	}

	printf("分配三个内存.\n");
	int *a = calloc(1, 8);
	int *b = calloc(1, 8);
	int *c = calloc(1, 8);

	printf("第一次1st分配calloc(1, 8)到: %p\n", a);
	printf("第二次2nd分配calloc(1, 8)到: %p\n", b);
	printf("第三次3rd分配calloc(1, 8)到: %p\n", c);

	printf("Free第一个内存...\n");
	free(a);

	printf("如果我们再次free %p, 程序会崩溃，因为 %p 在freelist的第一个.\n", a, a);
	// free(a);

	printf("所以我们来free第二块内存 %p.\n", b);
	free(b);

	printf("现在我们就可以再次free %p 了，因为他不是“上次”释放的内存了.\n", a);
	free(a);

	printf("现在free list为 [ %p, %p, %p ]. 如果我们malloc三次，我们会得到两次 %p !\n", a, b, a, a);
	a = calloc(1, 8);
	b = calloc(1, 8);
	c = calloc(1, 8);
	printf("第一次1st分配calloc(1, 8)到: %p\n", a);
	printf("第二次2nd分配calloc(1, 8)到: %p\n", b);
	printf("第三次3rd分配calloc(1, 8)到: %p\n", c);

	assert(a == c);
}
