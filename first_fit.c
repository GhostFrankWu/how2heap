#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
    * Glibc实现了Ptmalloc2来进行内存分配，其中最基本的内存管理单位成为chunk
    * 在64位中，chunk最小为32字节
    * Ptmalloc2将大小不同的free chunk分为：Fast Bin，Small Bin，Large Bin，Unsorted Bin
    * Fast Bin分类的chunk大小为32-128字节，以单链表存储，存取方式为LIFO
    * Small Bin分类的chunk大小为32-1024字节，以双链表存储，存取方式为FIFO
    * Large Bin存放大于1024(0X400)字节的chunk，以双链表存储，其中的指针fd与bk指向相同大小的前后chunk，不同大小的使用fd/bk_nextsize记录
    * Unsorted Bin是存放被free的chunk的垃圾桶。
    * 在申请新内存时，如果Unsorted Bin中有合适的chunk，则直接从Unsorted Bin中分配或分割，否则从Fast Bin/Small Bin/Large Bin中取出
*/

int main(){
	fprintf(stderr, "这份代码用于展示glibc的分配特性，不包含攻击内容\n");
	fprintf(stderr, "glibc使用first-fit（首次适应算法）分配chunk。\n");
	fprintf(stderr, "如果一个chunk足够大且可用, malloc就会分配这个chunk。\n");
	fprintf(stderr, "这个特性可用被use-after-free（UAF攻击）利用。\n");

	fprintf(stderr, "分配2个缓冲区。他们可以很大，不需要被分配在fastbin中。\n");
	char* a = malloc(0x512);
	char* b = malloc(0x256);
	char* c;

	fprintf(stderr, "第一次 1st malloc(0x512): %p\n", a);
	fprintf(stderr, "第二次2nd malloc(0x256): %p\n", b);
	fprintf(stderr, "我们可以继续分配很多这样的内存...\n");
	fprintf(stderr, "现在我们向其中写入一串字符 \"this is A!\" 以便读取\n");
	strcpy(a, "this is A!");
	fprintf(stderr, "第一次分配的空间 %p 写有内容 %s\n", a, a);

	fprintf(stderr, "free第一次分配的空间...\n");
	free(a);

	fprintf(stderr, "我们不需要再free任何东西，只要我们申请一个小于0x512大小的内存，他一定会位于 %p\n", a);

	fprintf(stderr, "那么，让我们分配 0x500个bytes\n");
	c = malloc(0x500);
	fprintf(stderr, "第三次 3rd malloc(0x500): %p\n", c);
	fprintf(stderr, "然后在第三次分配的空间中放一个不同的字符串 \"this is C!\"\n");
	strcpy(c, "this is C!");
	fprintf(stderr, "第三次分配的空间 %p 写有内容 %s\n", c, c);
	fprintf(stderr, "第一次分配的空间 %p 写有内容 %s\n", a, a);
	fprintf(stderr, "如果我们重新使用第一次分配的指针，会修改第三次分配If we reuse the first allocation, it now holds the data from the third allocation.\n");
}
