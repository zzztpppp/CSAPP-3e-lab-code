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

/* Enter debug mode, where mm_checkheap is invoked after ever operation */
#define DEBUG
#ifdef DEBUG
    #define mm_checkheap_d mm_checkheap()
#else
    #define mm_checkheap_d ((void)0)
#endif

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
#define GET_P(p) (*(size_t *)(p))
#define PUT_P(p, val)(*(size_t *)(p) = ((size_t)(val)))


/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given block ptr bp, compute value of pred and succ free blocks */
#define PRED_BLKP(bp) ((char *)(*(size_t *)(bp)))
#define SUCC_BLKP(bp) ((char *)(*(size_t *)((char *)(bp) + WSIZE)))

/* Given block ptr bp, compute address of pred and succ */
#define PREDP(bp) ((char *)(bp))
#define SUCCP(bp) ((char *)(bp) + WSIZE)

/* Compare whether the address a is higher than the address b */
# define ADDR_GTR(a, b) ((char *)(a) > (char *)(b))


/* Local helper functions */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void place(void *bp, size_t size);
static void *find_fit(size_t asize, void *heap_listp);
static void put_free(void *bp);
static void remove_free(void *bp);
static void mm_checkheap(void);
static void printblock(void *bp);
static void checkblock(void *bp);
static void checkheap(int verbose);



// Pointer pointing to the first free block
static void *free_listp;

static char *heap_listp;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    void *bp;

    // Create the initial empty heap
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    
    PUT(heap_listp, 0); // Alignment padding
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); // Prologue header
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); // Prologue footer
    PUT(heap_listp + (3*WSIZE), PACK(0, 1)); // Epilogue header
    
    heap_listp = heap_listp + (2*WSIZE);
    if ((bp = extend_heap(CHUNKSIZE/WSIZE)) == NULL)
        return -1;
    
    /* Initialize free_listp to contain the block that we just carved out */
    free_listp = bp;

    mm_checkheap_d;
    
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
    PUT_P(PREDP(bp), NULL);
    PUT_P(SUCCP(bp), NULL);

    /* Put the newly accquired chunk at the end  of the free list.*/
    put_free(bp);

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

        // Toggle predecessor and successor
        // empty block pointer
        PUT_P(SUCCP(bp), SUCC_BLKP(next_bp)); /* Successor of new bp */
        PUT_P(PREDP(SUCC_BLKP(next_bp)), bp); /* Predecessor of the successor */

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */

        PUT_P(SUCCP(prev_bp), SUCC_BLKP(bp));
        PUT_P(PREDP(SUCC_BLKP(bp)), prev_bp);

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = prev_bp;
    }

    else { /* Case 4 */

        PUT_P(SUCCP(prev_bp), SUCC_BLKP(next_bp));
        PUT_P(PREDP(SUCC_BLKP(next_bp)), prev_bp);

        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
        GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

    }

    mm_checkheap_d;

    return bp;
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

    char *bp = free_listp;
    while ((!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) < asize))){
        bp = SUCC_BLKP(bp);
    }

    return bp;
}

/*
 * place - Take up a given size of chunks of a block bp. 
 *     Put the fragment(if any) into the corresponding free list.
 */ 
void place(void *bp, size_t size){
    size_t block_size = GET_SIZE(HDRP(bp));
    size_t csize = block_size - size;

    /* Remove the block from free list */
    remove_free(bp);

    if (csize >= 2*DSIZE){
        // There is a fragmentation, need to make the residual space a block
        PUT(HDRP(bp), PACK(size, 1)); // Header of the allocated block
        PUT(FTRP(bp), PACK(size, 1)); // Footer of the allocated block

        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize, 0));
        PUT(FTRP(bp), PACK(csize, 0));

        /* Put the residual block into free list */
        put_free(bp);

    }
    else{
        // No fragmentation
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));
    }
    
    mm_checkheap_d;
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    put_free(bp);
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

/**********************************
 * Helper functions
 **********************************/

/*
 * put_free - Put a untracked free block bp at the 
 * right place of the free list upon address order.
 */ 
static void put_free(void *bp)
{
    if (free_listp == NULL){
        /* The free list is empty */
        free_listp = bp;
        return;
    }

    /* Check if the block to insert has the lowest address order */
    if (ADDR_GTR(free_listp, bp)){
        PUT_P(SUCCP(bp), free_listp);
        PUT_P(PREDP(free_listp), bp);
        free_listp = bp;
        return;
    }
     
    char *node_bp = free_listp;
    /* Find the predecessor of bp */   
    while (!ADDR_GTR(bp, node_bp)){
        node_bp = SUCC_BLKP(node_bp);
    }

    /* Insert bp between node_bp and succ_bp, if any.*/
    void *succ_bp = SUCC_BLKP(node_bp);
    if (succ_bp != NULL){
        PUT_P(PREDP(succ_bp), bp);
    }
    PUT_P(SUCCP(node_bp), bp);
    PUT_P(PREDP(bp), node_bp);
    PUT_P(SUCCP(bp), succ_bp);

    return;
}


/*
 * remove_free - Remove a block free block from the free_list 
 *     for use.
 */
static void remove_free(void *bp){
    void *pred_bp = PRED_BLKP(bp);
    void *succ_bp = SUCC_BLKP(bp);

    if (pred_bp != NULL)
        PUT_P(SUCCP(pred_bp), succ_bp);

    if (succ_bp != NULL) 
        PUT_P(PREDP(succ_bp), pred_bp);
   
    /* We are removing the fisrt block */ 
    if (pred_bp == NULL)
        free_listp = succ_bp;
    
    return;
}


/************************************************************* 
 * Heap consistency checker
 **************************************************************/

/*
 * isin_free - Given plock pointer bp
 *     return 1 if bp is in free block, 0 otherwise.
 */
static int isin_free(void *bp){
    if (free_listp == NULL)
        return 0;
    
    void *current_bp = free_listp;
    while (current_bp != NULL){
        if (current_bp == bp) return 1;
        current_bp = SUCC_BLKP(current_bp);
    }

    return 0;
}


/*
 * mm_checkheap - Checks heap consistency:
 *     whether blocks are continues, whether there is no adjacent
 *     free blocks, whether all free blocks in the free list
 *     are adress ordered, whether all blocks in the heap_list
 *     marked as free are in free_list, whether all blocks in the
 *     free list are marked as free in heap list.
 */
static void mm_checkheap(void){
    char *bp;
    checkheap(0);
    if (free_listp != NULL){
        
        /* Check free list to be address ordered */
        bp = free_listp;
        while ((bp = SUCC_BLKP(free_listp)) != NULL){
           if (ADDR_GTR(PRED_BLKP(bp), bp))
               fprintf(stderr, "Free block %p should not predecede free block  %p",
                       PRED_BLKP(bp), bp);
        }

        /* Check that blocks presenting at free list are marked as free in heap list */
        bp = free_listp;
        while (bp != NULL){
            if (GET_ALLOC(HDRP(bp)))
                fprintf(stderr, "Block %p in free list but is allocated.\n", bp);
            bp = SUCC_BLKP(bp);
        }
    }

    /* Check that blocks in heap list marked as free are in free list */
    for (bp = heap_listp; (GET_SIZE(HDRP(bp)) > 0) && (!GET_ALLOC(HDRP(bp))); bp = NEXT_BLKP(bp)){
        if (!isin_free(bp)){
            fprintf(stderr, "Block %p markded as free but not found in free list.\n",
                    bp);
        }
    }
}

static void printblock(void *bp) 
{
    size_t hsize, halloc, fsize, falloc;

    checkheap(0);
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));  
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));  

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%ld:%c] footer: [%ld:%c]\n", bp, 
           hsize, (halloc ? 'a' : 'f'), 
           fsize, (falloc ? 'a' : 'f')); 
}
 

static void checkblock(void *bp) 
{
    if ((size_t)bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
        printf("Error: header does not match footer\n");
}

/* 
 * checkheap - Minimal check of the heap for consistency 
 */
void checkheap(int verbose) 
{
    char *bp = heap_listp;

    if (verbose)
        printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");
    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose) 
            printblock(bp);
        checkblock(bp);
    }

    if (verbose)
        printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");
}