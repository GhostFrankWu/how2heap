#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// 需要glibc<=2.27
// export LD_PRELOAD="./glibc-all-in-one/libs/2.23-0ubuntu3_amd64/libc.so.6"
/**
 *大于0x80的chunk被释放之后就放到了unsortedbin上面去，
 *但是unsortedbin是一个未分类的bin，上面的chunk也处于未分类的状态
 *这些chunk需要在特定的条件下被整理然后放入到smallbins或者largebins中
 *这个整理的过程称为unsortedbin的“consolidate”
 *而consolidate”是要在特定的条件下才会发生的，并且与malloc紧密相关
 */
int main227() {
  void* p1 = malloc(0x40);
  void* p2 = malloc(0x40);
  fprintf(stderr, "malloc两个fastbin: p1=%p p2=%p\n", p1, p2);
  fprintf(stderr, "释放p1!\n");
  free(p1);

  void* p3 = malloc(0x400);
  // 此处当我们malloc一个chunk，这个chunk的大小大于smallbin的范围，在unsortedbin中寻找可用的freechunk使用时，会触发malloc_conlolidate
  fprintf(stderr, "申请一个比较大(0x400)的内存来触发malloc_consolidate(): p3=%p\n", p3);
  fprintf(stderr, "在这次 malloc_consolidate()中, p1 被移动的 unsorted bin.\n");
  free(p1);
  fprintf(stderr, "再次释放p1，产生double free!\n");
  fprintf(stderr, "我们可以绕过malloc()的检查，因为此时p1并不在fast的顶端.\n");
  fprintf(stderr, "现在p1既在unsorted bin又在fast bin. 所以两次malloc都会得到p1: %p %p\n", malloc(0x40), malloc(0x40));
}
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
// 需要glibc<=2.27，但现在不用了
void main() {
	// reference: https://valsamaras.medium.com/the-toddlers-introduction-to-heap-exploitation-fastbin-dup-consolidate-part-4-2-ce6d68136aa8
	puts("这是一个用来绕过tcachebin中double free检查的强大技巧.");
	printf("填满tcache来强制启用fast bin...\n");

	void *ptr[7];

	for(int i = 0; i < 7; i++)
		ptr[i] = malloc(0x40);
	for(int i = 0; i < 7; i++)
		free(ptr[i]);

	void* p1 = calloc(1,0x40);

	printf("Allocate 一个同样大小的chunk p1=%p \n", p1);
  	printf("这时候free p1，它会进入fastbin...\n\n");
  	free(p1);

  	void* p3 = malloc(0x400);
	printf("Allocate 一个大小为 tcache-sized 的 chunk (p3=%p)\n", p3);
	printf("申请一个比较大(0x400)的内存会触发malloc_consolidate，将fastbin合并到top chunk中，现在p1 and p3指向相同的chunk!\n\n");

	assert(p1 == p3);

	fprintf(stderr, "再次释放p1，产生double free!\n");
	free(p1);

	void *p4 = malloc(0x400);

	assert(p4 == p3);

	printf("double free把p1加到了tcache，这样再分配同样大小的块就会指向p3 \n");
	printf("现在p1既在unsorted bin又在fast bin. 所以两次malloc都会得到p1(p3): p3=%p, p4=%p\n\n",p3, p4);
}
