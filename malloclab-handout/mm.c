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

/* Read and write two words at address p */
#define GET_P(p) (*(unsigned long *)(p))
#define PUT_P(p, val)(*( unsigned long *)(p) = (val))


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
#define PRED_BLKP(bp) (*(unsigned long *)(bp)) 
#define SUCC_BLKP(bp) (*(unsigned long *)((char *)(bp) + DSIZE))


/* Local helper functions */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void place(bp, size);
static void *find_fit(size_t asize, void *heap_listp);

// Linked list that contains free lists of different size class.
static void **free_lists;

static void *heap_listp;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    void *bp;

    // Initialize the empty free lists
    free_lists = NULL;

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

    /* Initialize free block sucessor and predeccessor */
    PUT_P(bp, NULL);
    PUT_P(bp + DSIZE, NULL);

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
        PUT_P(bp + DSIZE, SUCC_BLKP(next_bp));
        PUT_P(NEXT_BLKP(next_bp), bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

        PUT_P(prev_bp + DSIZE, SUCC_BLKP(bp));
        PUT(next_bp, prev_bp);
    }

    else { /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
        GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

        PUT_P(prev_bp + DSIZE, SUCC_BLKP(next_bp));
        PUT_P(NEXT_BLKP(next_bp), prev_bp);
    }
        return bp;
}


/*
 * insert_free_list - Insert the free block into
 *     free_listp. Blocks are put as address-ordered.
 */
static void insert_free_list(void *bp, void *free_listp){

    // Target free list is empty, populate it with the bp.
    if (free_listp == NULL)
       return;

    void *current_bp = free_listp;
    void *succ_bp = SUCC_BLKP(current_bp);
    while (current_bp != NULL){
        if (bp > current_bp){
            current_bp = succ_bp;
            succ_bp = SUCC_BLKP(current_bp);
        }
        else if(bp < current_bp){
            SUCC_BLKP(current_bp) = bp;
            PRED_BLKP(bp) = current_bp;
            SUCC_BLKP(bp) = succ_bp;
            if (succ_bp != NULL)
                PRED_BLKP(succ_bp) = bp;
            return;
        }
        else{
            fprintf(stderr, "ERROR: Heap currupted. Duplicated blocks %p, %p found", (void *)bp, (void *)current_bp);
        }
    }

    fprintf(stderr, "ERROR: Can't insert block %p, into %p", (void *)bp, (void *)heap_listp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    char *bp;
    size_t extendsize;

    // Ignore suprious request
    if (size == 0)
        return NULL;

    int newsize = ALIGN(size + SIZE_T_SIZE);

    // Search the free list for a hit.
    if ((bp = find_fit(newsize, heap_listp)) == NULL){
        place(bp, newsize);
        return bp;
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
        bp = SUCC_BLKP(bp);
    }

    return bp;
}

/*
 * place - Take up a given size of chunks of a block bp. 
 *     Put the fragment(if any) into the corresponding free list.
 */ 
void place(bp, size){
    size_t block_size = GET_SIZE(bp);
    size_t csize = block_size - size;

    if (csize > 2*DSIZE){
        // There is a fragmentation, need to make the residual space a block
        PUT(HDRP(bp), PACK(size, 1)); // Header of the allocated block
        PUT(FTRP(bp), PACK(size, 1)); // Footer of the allocated block

        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize, 0));
        PUT(FTRP(bp), PACK(csize, 0));

        // New block inherit sucessor and predeccessor from old block
        PUT_P(bp, PRED_BLKP(PREV_BLKP(bp)));
        PUT_P(bp + DSIZE, SUCC_BLKP(PREV_BLKP(bp)));
    }
    else{
        // No fragmentation
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));
    }
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
