/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4 /* Word and header/footer size (bytes) */
#define DSIZE 8 /* Double word size (bytes) */
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given block ptr bp, compute address of pred and succ free blocks */
#define PRED_BLKP(bp) (*(long *)(bp)) 
#define SUCC_BLKP(bp) (*(long *)(bp + DSIZE))

/* Given block ptr bp, cut off its relationship with successor or 
 * predecessor
 */

/* Put 2 words at address p.*/
#define PUT_BLKPTR(p, val) (*(long *)(p) = (val))


static const int num_free_list = 22;

// Each free list contains  2^i < size  <= 2^(i+1) of free blocks
static void *free_lists_array[num_free_list];

static void *heap_listp;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    void *bp;

    // Initialize the empty free list array
    for (int i=0; i < num_free_list; i++){
        free_lists_array[i] = NULL;
    }

    // Create the initial empty heap
    if ((heap_listp = mem_sbrk(6*WSIZE)) == (void *)-1)
        return -1;
    
    PUT(heap_listp, 0); // Alignment padding
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); // Prologue header
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); // Prologue footer
    PUT(heap_listp + (3*WSIZE), PACK(DSIZE, 1)); // Epilogue header
    
    heap_listp = heap_listp + (2*WSIZE);
    
    if ((bp = extend_heap(CHUNKSIZE/WSIZE)) == NULL)
        return -1;
    
    // Put the first block into right size class
    put_block(bp);

    return 0;
}

/* extend_heap - Extend heap list by words.
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); /* Free block header */
    PUT(FTRP(bp), PACK(size, 0)); /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/* coalesce - Coalesce two adjacent emply blocks.
 *  
 */
static void *coalesce(void *bp)
{
    void *prev_bp = PREV_BLKP(bp);
    void *next_bp = NEXT_BLKP(bp);

    size_t prev_alloc = GET_ALLOC(FTRP(prev_bp));
    size_t next_alloc = GET_ALLOC(HDRP(next_bp));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) { /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));

        // Toggle predecessor and successor
        // empty block pointer
        SUCC_BLKP(bp) = SUCC(next_bp);
        PRED_BLKP(NEXT_BLKP(next_bp)) = bp;
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

        SUCC_BLKP(prev_bp) = SUCC_BLKP(bp);
        PRED_BLKP(next_bp) = prev_bp;
    }

    else { /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
        GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

        SUCC_BLKP(prev_bp) = SUCC_BLKP(next_bp);
        PRED_BLKP(NEXT_BLKP(next_bp)) = prev_bp;
    }
        return bp;
}


/* put_block - Put the free block bp at the right place of free_list_array
 *   according to the block size.
 */
static void put_block(void *bp){
    size_t size  = GET_SIZE(HDRP(bp)) / DSIZE;
    int size_class;
    
    // Find the heap list of  proper size class 
    size_class = get_size_class(size);
    heap_listp = free_lists_array[size_class];

    // Insert the block into free list of the size class
    insert_free_list(bp, heap_listp);

}

/*
 * insert_free_list - Insert the free block into
 *     free_listp. 
 */
static void insert_free_list(void *bp, void *free_listp){
    void *succ_blkp = free_listp;
    while (succ_blkp != NULL){
        succ_blkp = SUCC_BLKP((char *)(succ_blkp));
    }

    // Append the the end of the free list. 
    succ_blkp = bp; 
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    char *bp;
    size_t extendsize;
    int size_class;

    // Ignore suprious request
    if (size == 0)
        return NULL;

    int newsize = ALIGN(size + SIZE_T_SIZE);
    size_class = get_size_class(newsize);

    // Search the free list for a hit.
    while(size_class < num_free_list){
        heap_listp = free_lists_array[size_class];
        if ((bp = find_fit(newsize, heap_listp)) == NULL){
            place(bp, newsize);
            return bp;
        }

        // No hit in current size class. Go for one bigger.
        size_class += 1;
    }

    // No fit found. Get more memory and place the block.
    extendsize = MAX(newsize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    
    place(bp, newsize);
    return bp;
}

/*
 * find_fit - Find the free list that has size of at least
 *     asize. Search the free list, return the first fit block.
 */
static void *find_fit(size_t asize, void *heap_listp){

    char *bp = heap_listp;
    while ((!GET_ALLOC(bp) && (GET_SIZE(bp) < asize))){
        bp = NEXT_BLKP(bp);
    }

    return bp;
}

/*
 * get_size_class - Return the index of free_lists_array where 
 *      the indexed free_list (if not empty) has blocks of size 
 *      at least size.
 */ 
static int get_size_class(size_t size){

    int size_class;
    size = size/WSIZE;
    for(int i=0; i < num_free_list; i++){
         if (size <= (1 << i)){
             size_class = i;
             break;
         }
    }
    
    return size_class;
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    bp = coalesce(bp);
    put_block(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}


/* 
 * Helper function for accessing the free_list data structure
 */

