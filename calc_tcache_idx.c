#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

/*这是Ptmalloc2中chunk的基本结构*/
struct malloc_chunk {

  size_t      mchunk_prev_size;  /* Size of previous chunk (if free).  */
  size_t      mchunk_size;       /* 当前chunk的大小 */
  /* 因为chunk一定是8字节对齐，所以mchunk_size的低3位为0，它们作为flag位分别表示：
   * PREV_INUSE：前一个chunk是否被分配，1表示已经分配，0表示被释放
   * IS_MMAPPED：当前chunk是否是mmap分配的，1表示是mmap分配的，0表示是malloc分配的
   * NON_MAIN_ARENA：当前chunk是否是main_arena（主线程）的，1表示属于main_arena
   */

  /* 在small bin和large bin中使用。  */
  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;

  /* 只在large bin中使用，形成排序链表。  */
  struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
  struct malloc_chunk* bk_nextsize;
};

/* 当前系统中一个 word 的大小  */
#define SIZE_SZ (sizeof (size_t))

#define MALLOC_ALIGNMENT (2 * SIZE_SZ < __alignof__ (long double) \
			  ? __alignof__ (long double) : 2 * SIZE_SZ)

/* The corresponding bit mask value.  */
#define MALLOC_ALIGN_MASK (MALLOC_ALIGNMENT - 1)

/* The smallest possible chunk */
#define MIN_CHUNK_SIZE        (offsetof(struct malloc_chunk, fd_nextsize))

/* The smallest size we can malloc is an aligned minimal chunk */
#define MINSIZE  \
  (unsigned long)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))

#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

/* When "x" is from chunksize().  */
# define csize2tidx(x) (((x) - MINSIZE + MALLOC_ALIGNMENT - 1) / MALLOC_ALIGNMENT)

/* When "x" is a user-provided size.  */
# define usize2tidx(x) csize2tidx (request2size (x))

int main()
{
    unsigned long long req;
    unsigned long long tidx;
	fprintf(stderr, "这段代码在给定chunk size时计算了tcache idx，不演示攻击。\n");
	fprintf(stderr, "计算公式如下:\n");
    fprintf(stderr, "\tIDX = (CHUNKSIZE - MINSIZE + MALLOC_ALIGNMENT - 1) / MALLOC_ALIGNMENT\n");
    fprintf(stderr, "\t在当前的系统（x64）下，这些值是:\n");
    fprintf(stderr, "\t\tMINSIZE: 0x%lx\n", MINSIZE);
    fprintf(stderr, "\t\tMALLOC_ALIGNMENT: 0x%lx\n", MALLOC_ALIGNMENT);
    fprintf(stderr, "\t所以我们得出如下公式:\n");
    fprintf(stderr, "\tIDX = (CHUNKSIZE - 0x%lx) / 0x%lx\n\n", MINSIZE-MALLOC_ALIGNMENT+1, MALLOC_ALIGNMENT);
    fprintf(stderr, "但请注意 CHUNKSIZE 并不是 malloc(x)中的x\n");
    fprintf(stderr, "x的计算方式如下:\n");
    fprintf(stderr, "\tIF x + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE(0x%lx) CHUNKSIZE = MINSIZE (0x%lx)\n", MINSIZE, MINSIZE);
    fprintf(stderr, "\tELSE: CHUNKSIZE = (x + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK) \n");
    fprintf(stderr, "\t=> CHUNKSIZE = (x + 0x%lx + 0x%lx) & ~0x%lx\n\n\n", SIZE_SZ, MALLOC_ALIGN_MASK, MALLOC_ALIGN_MASK);
    while(1) {
        fprintf(stderr, "[按下CTRL-C退出] 请输入16进制大小x用于malloc(x)，比如0x10: ");
        scanf("%llx", &req);
        tidx = usize2tidx(req);
        if (tidx > 63) {
            fprintf(stderr, "\nWARNING: 超出TCACHE范围!\n");
        }
        fprintf(stderr, "\nTCache Idx: %llu\n", tidx);
    }
    return 0;
}
