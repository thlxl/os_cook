#include "mem.h"

/* 堆大小不要太小 */
#define MINIMUM_BLOCK_SIZE	((size_t)( block_node_size << 1))
#define BITS_PER_BYTE       (( size_t)8)

static u8_t os_mem[ configTOTAL_HEAP_SIZE ];

/*块链表节点，用来链接空闲内存块的*/
typedef struct block_node
{
    struct block_node *nxt_free_block;  // 下一个空闲块
    size_t block_size;

}block_node_t;

/*块链表节点大小*/
static const size_t block_node_size	= (sizeof(block_node_t) + ((size_t)(configALIGNMENT - 1))) & ~(( size_t )configALIGNMENT_MASK);

/*空闲块链表头，尾*/
static block_node_t b_start, *b_end = NULL;

/*剩余可用内存字节数*/
static size_t b_FreeBytesRemaining = 0U;

/*当前最小空闲块*/
static size_t b_MinimumEverFreeBytesRemaining = 0U;

static size_t b_BlockAllocatedBit = 0;

static void os_memInit( void );
static void os_insertBlockIntoFreeList( block_node_t * pxBlockToInsert );

void *os_malloc( size_t wantedSize)
{
    block_node_t *block, *previousBlock, *newBlockLink;
    void * ret = NULL;

    os_suspendAllTask();
    {
        /*如果是第一次分配内存，则先要对空闲块进行初始化*/
        if( b_end == NULL)
        {
            os_memInit();
        }

        /*检查想要申请的内存是否太大*/
        if( ( wantedSize & b_BlockAllocatedBit ) == 0)
        {
            if( wantedSize > 0 )
            {
                /*需要申请的内存加上块结构体的大小才是真正分配的空闲块大小*/
                wantedSize += block_node_size;

                if( ( wantedSize & configALIGNMENT_MASK) != 0x00 )
                {
                    // 如果不是8字节对齐，则向上对齐到8字节
                    wantedSize += ( configALIGNMENT - ( wantedSize & configALIGNMENT_MASK ) );
                }
            }

            if( ( wantedSize > 0 ) && ( wantedSize <= b_FreeBytesRemaining ) )
            {
                /*如果当前有足够的内存块*/

                previousBlock = &b_start;
                block = b_start.nxt_free_block;
                /*寻找合适的空闲块,first fit算法*/
                while ( ( block->block_size < wantedSize ) && ( block->nxt_free_block != NULL ) )
                {
                    previousBlock = block;
                    block = block->nxt_free_block;
                }

                
                if( block != b_end )
                {
                    /*如果block不等于b_end，则找到足够大小的块*/
                    ret = ( void * ) (((u8_t *) previousBlock->nxt_free_block ) + block_node_size );

                    /*移除当前所找的块*/
                    previousBlock->nxt_free_block = block->nxt_free_block;

                    /*如果block分配之后的剩余大小大于MINIMUM_BLOCK_SIZE，则进行分割*/
                    if((block->block_size - wantedSize ) > MINIMUM_BLOCK_SIZE )
                    {
                        /*这里是如果原空闲块大小分配之后的剩余内存过小的话，后续也无法使用了，就直接全都给分配出去
                        * 只有剩余内存大于MINIMUM_BLOCK_SIZE时才把剩下的重新插入链表
                        */
                        newBlockLink = (void *) (((u8_t*) block ) + wantedSize );

                        newBlockLink->block_size = block->block_size - wantedSize;
                        block->block_size = wantedSize;

                        os_insertBlockIntoFreeList( newBlockLink );
                    }

                    /*更新剩余可用内存字节数*/
                    b_FreeBytesRemaining -= block->block_size;
                    if( b_FreeBytesRemaining < b_MinimumEverFreeBytesRemaining )
                    {
                        b_MinimumEverFreeBytesRemaining = b_FreeBytesRemaining;
                    }

                    /*设置块的标志位，表示已分配*/
                    block->block_size |= b_BlockAllocatedBit;
                    block->nxt_free_block = NULL;
                }
                
            }
        }
    }
    os_resumeAllTask();
	return ret;
}

void os_free( void *addr )
{
    u8_t *ptr = ( u8_t * ) addr;
    block_node_t *blockToFree;

	if(addr != NULL)
	{
		/* 减去结构体大小获得真正的释放地址 */
		ptr -= block_node_size;

		/* 内存块结构体指针 */
		blockToFree = (void *) ptr;

        /* 检查块是否是os_malloc分配 */
		if( (blockToFree->block_size & b_BlockAllocatedBit) != 0 )
		{
			if( blockToFree->nxt_free_block == NULL )
			{
				/*去掉标志位，获得真实大小*/
				blockToFree->block_size &= ~b_BlockAllocatedBit;
				os_suspendAllTask();
				{
					/* 将此块添加到空闲块链表中 */
					b_FreeBytesRemaining += blockToFree->block_size;
					os_insertBlockIntoFreeList( blockToFree );
				}
				os_resumeAllTask();
			}
		}
    }	
}

static void os_memInit( void )
{
    block_node_t *firstFreeBlock;
    u8_t *alignedMem;
    size_t address;
    size_t totalMemSize = configTOTAL_HEAP_SIZE;

    /*堆开始地址*/
    address = ( size_t ) os_mem;

    /*确保堆从正确的边界对齐*/
    if((address & configALIGNMENT_MASK) != 0)
    {
        address += (configALIGNMENT - 1 );
        address &= ~((size_t)configALIGNMENT_MASK );
        totalMemSize -= address - (size_t)os_mem;
    }

    alignedMem = (u8_t *) address;

    /*初始化空闲块链表头*/
    b_start.nxt_free_block = (void *) alignedMem;
    b_start.block_size = (size_t) 0;

    /*计算堆结束地址*/
    address = ((size_t)alignedMem) + totalMemSize;
    address -= block_node_size;
    address &= ~((size_t)configALIGNMENT_MASK);
    b_end = (void *) address;
    b_end->block_size = 0;
    b_end->nxt_free_block = NULL;

    /*初始化第一个空闲块*/
    firstFreeBlock = (void *) alignedMem;
    firstFreeBlock->block_size = address - (size_t) firstFreeBlock;
    firstFreeBlock->nxt_free_block = b_end;

    b_MinimumEverFreeBytesRemaining = firstFreeBlock->block_size;
    b_FreeBytesRemaining = firstFreeBlock->block_size;

    /*设置最高位标志，0x80000000，如果最高位为1表示块已分配*/
    b_BlockAllocatedBit = ((size_t) 1) << ((sizeof(size_t) * BITS_PER_BYTE) - 1);

}

static void os_insertBlockIntoFreeList( block_node_t * blockToInsert )
{
    block_node_t *iterator;
    u8_t *ptr;

    /*按照地址顺序插入空闲块*/
    for( iterator = &b_start; iterator->nxt_free_block < blockToInsert; iterator = iterator->nxt_free_block )
    {
        /*什么都不需要做，只需要把指针迭代到正确位置*/
    }

    ptr = (u8_t*)iterator;
    /*如果blockToInsert的起始地址紧挨着iterator的结束地址，说明这是连续空闲块则合并*/
    if( (ptr + iterator->block_size ) == ( u8_t * ) blockToInsert )
    {
        iterator->block_size += blockToInsert->block_size;
        blockToInsert = iterator;
    }

    ptr = (u8_t*)blockToInsert;
    /*如果blockToInsert的结束地址紧挨着iterator的下一个空闲块的起始地址，说明这是连续空闲块则合并*/
    if( (ptr + blockToInsert->block_size ) == ( u8_t * ) iterator->nxt_free_block)
    {
        if( iterator->nxt_free_block != b_end)
        {
            /*如果下一个空闲块不是b_end，则合并,且下一个空闲块指向iterator->nxt_free_block->nxt_free_block*/
            blockToInsert->block_size += iterator->nxt_free_block->block_size;
            blockToInsert->nxt_free_block = iterator->nxt_free_block->nxt_free_block;
        }
        else
        {
            /*如果下一个空闲块是b_end，则将blockToInsert的nxt_free_block指向b_end*/
            blockToInsert->nxt_free_block = b_end;
        }
        
    }
    else
    {
        /*如果没有合并的，直接插入*/
        blockToInsert->nxt_free_block = iterator->nxt_free_block;
    }

    if( iterator != blockToInsert )
    {
        /*如果和上一个也没合并，就把上一个的nxt_free_block指向blockToInsert*/
        iterator->nxt_free_block = blockToInsert;
    }
}

