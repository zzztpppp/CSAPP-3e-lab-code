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
    #define mm_checkheap_d printf("Check heap at %d, %s\n", __LINE__, __FILE__);mm_checkheap();
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
static void coalesce_free(void *bp);
static int is_block(void *bp);
static void carve(void *bp, size_t csize);



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

    free_listp = NULL;
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

    /* Coalesce adjacent blocks of bp in free list first */
    coalesce_free(bp);

    if (prev_alloc && next_alloc) { /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = prev_bp;
    }

    else { /* Case 4 */

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
 * coalesce_free - Coalesce adjacent blocks in free list.
 */
static void coalesce_free(void *bp){
    /* Blocks that are adjacent to bp in free list */
    char *succ_bp = SUCC_BLKP(bp);
    char *pred_bp = PRED_BLKP(bp);

    /* Blocks that are adjacent to bp in heap list */
    char *next_bp = NEXT_BLKP(bp);
    char *prev_bp = PREV_BLKP(bp);

    /* If both conditions are hit, then its case 4. If none is hit, its case 1 */
    if (succ_bp == next_bp){
        remove_free(succ_bp);  /* Case 2 */ 
    }

    if (prev_bp == pred_bp){
        remove_free(bp);      /* Case 3 */
    }

    return;
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
    if ((bp = find_fit(newsize, heap_listp)) != NULL){
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
    while ((bp != NULL) && (GET_ALLOC(HDRP(bp)) || (GET_SIZE(HDRP(bp)) < asize))){
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
 * mm_realloc - Realloc memeory of size to the given ptr.
 *     It firt tries to extend the ptr by accquiring more
 *     space from the adjacent empty blocks. If failed, 
 *     fall back to mm_malloc(size)
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copysize;
    size_t oldsize;
    size_t newsize;
    size_t csize;

    if (size == 0){
        mm_free(ptr);
        return ptr;
    }

    if (ptr == NULL){
        return mm_malloc(size);
    }

    /* Check if ptr is a valid block pointer returned by ealier calls
     * to mm_malloc and mm_realloc 
     */
    if (!is_block(ptr)){
        fprintf(stderr, "Invalid pointer %p\n", ptr);
        exit(134);
    }
    
    /* Reallocate a valid block, eithre to extend or to shrink the block size */
    oldsize = GET_SIZE(HDRP(oldptr));
    newsize = ALIGN(size + SIZE_T_SIZE);
    if (newsize > oldsize){    /* Extend the block */
        char *prev_bp = PREV_BLKP(oldptr);
        char *next_bp = NEXT_BLKP(oldptr);

        size_t prev_size = GET_SIZE(HDRP(prev_bp));
        size_t next_size = GET_SIZE(HDRP(next_bp));

        int prev_alloc = GET_ALLOC(HDRP(prev_bp));
        int next_alloc = GET_ALLOC(HDRP(next_bp));

        csize = newsize - oldsize;

        if ((!next_alloc) && (next_size + oldsize >= newsize)){
            /* Extend the block via the right free block */
            PUT(HDRP(oldptr), PACK(next_size + oldsize, 1));
            PUT(FTRP(oldptr), PACK(next_size + oldsize, 1));

            carve(oldptr, newsize - next_size - oldsize);
            newptr =  oldptr;
        }
        else if (!prev_alloc && (oldsize + prev_size >= newsize)){
            /* Extend the block via the left free block */
            PUT(HDRP(prev_bp), PACK(prev_size + oldsize, 1));
            PUT(FTRP(prev_bp), PACK(prev_size + oldsize, 1));

            carve(prev_bp, newsize - prev_size - oldsize);
            memcpy(prev_bp, oldptr, oldsize - SIZE_T_SIZE);
            newptr = prev_bp;
        }
        else if ((!prev_alloc && !next_alloc) && (oldsize + prev_size + next_size >= newsize)){
            /* Extend the block via left & right free block */
            PUT(HDRP(prev_bp), PACK(prev_size + oldsize + next_size, 1));
            PUT(FTRP(prev_bp), PACK(prev_size + oldsize + next_size, 1));

            carve(prev_bp, newsize - prev_size - oldsize - next_size);
            memcpy(prev_bp, oldptr, oldsize - SIZE_T_SIZE);
            newptr = prev_bp;
        }
        else{
            /* Can't extend the block, find another block */
            newptr = mm_malloc(size);
            memcpy(newptr, oldptr, oldsize - SIZE_T_SIZE);
            mm_free(oldptr);
        }
    }
    else{    /* Shrink the block or do nothing */
        csize = oldsize - newsize;
        carve(oldptr, csize);
        newptr = oldptr;
    }

    mm_checkheap_d;
    return newptr;
}

/**********************************
 * Helper functions
 **********************************/


/*
 * carve - Carve out a csize-sized free block from the
 *    the given block bp.
 */
static void carve(void *bp, size_t csize){
    size_t size = GET_SIZE(HDRP(bp));
    if(csize < 2*DSIZE){
        return;
    }
    else{
        PUT(HDRP(bp), PACK(size - csize, 1));
        PUT(FTRP(bp), PACK(size - csize, 1));

        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize, 0));
        PUT(FTRP(bp), PACK(csize, 0));

        put_free(bp);
        coalesce(bp);
        return;
    }
}

/*
 * isin_block - Return 1 if the given block pointer
 *     ptr is a valid block returned by previous mm_malloc or mm_realloc.
 */
static int is_block(void *ptr){
    for (void *bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
        if (bp == ptr) return 1;
    }

    return 0;
}

/*
 * put_free - Put a untracked free block bp at the 
 * right place of the free list upon address order.
 */ 
static void put_free(void *bp)
{
    /* Clean up the payload for succp and predp */
    PUT_P(SUCCP(bp), NULL);
    PUT_P(PREDP(bp), NULL);

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
    void *succ_bp = SUCC_BLKP(node_bp);

    /* Find the predecessor of bp */   
    while (ADDR_GTR(bp, node_bp)){
        if ((succ_bp == NULL) || ADDR_GTR(succ_bp, bp)) 
            break;
        node_bp = succ_bp;
        succ_bp = SUCC_BLKP(node_bp);
    }

    /* Insert bp between node_bp and succ_bp, if any.*/
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
    char *next_bp;
    checkheap(0);
    if (free_listp != NULL){
        
        /* Check free list to be address ordered */
        printf("Check that free blocks are address-ordered.\n");
        for (bp = free_listp; (next_bp = SUCC_BLKP(bp)) != NULL; bp=next_bp){
            // printblock(bp);
           if (ADDR_GTR(PRED_BLKP(bp), bp))
               printf( "Free block %p should not predecede free block  %p",
                       PRED_BLKP(bp), bp);
        }

        /* Check that blocks presenting at free list are marked as free in heap list */
        printf("Check free block false negatives\n");
        for (bp = free_listp; bp != NULL; bp = SUCC_BLKP(bp)){
            // printblock(bp);
            if (GET_ALLOC(HDRP(bp)))
                printf("Block %p in free list but is allocated.\n", bp);
        }
    }

    /* Check that blocks in heap list marked as free are in free list */
    for (bp = heap_listp; (!GET_ALLOC(HDRP(bp))) && (GET_SIZE(HDRP(bp)) > 0); bp = NEXT_BLKP(bp)){
        printf("Checking free blocks false positive.\n");
        if (!isin_free(bp)){
            printf("Block %p markded as free but not found in free list.\n",
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