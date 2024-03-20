#include <stdio.h>
#include <stdbool.h>
#define HEAP_SIZE 1024
#define ALLOWED_EXCESS_SIZE 5

char myHeap[HEAP_SIZE];

/**
 * @brief : status signifies whether block is free or allocated
 */
typedef enum
{
    ALLOCATED,
    FREE
} status;

/**
 * @brief : Structure representing the meta data of a free block of memory.
 * @details : Contains status and size of current block, pointers to adjacent free blocks. (16 bytes)
 */
typedef struct metaData_tag
{
    status isFree;             /*< isFree signifies whether the block is free or allocated >*/
    int size;                  /*< Size of the memory block >*/
    struct metaData_tag *prev; /*< Pointer to the previous memory block >*/
    struct metaData_tag *next; /*< Pointer to the next memory block >*/
} metaData;

/**
 * @brief : Structure to define the one free list.
 * @details : It is important to have a separate structure to define a freelist as it allows us to keep a track of whether or not the list has been initialized
 * without it the user has to manually initialise the free list in the main function and it also risks multiple initializations in the same program.
 * Using this structure we can always check 'freeList->isInit' to find out whether the free list has been initialized and therefore ensure encapsulation and mitigate the above mentioned list.
 * @note : freeList is a global variable.
 */
struct freeList_tag
{
    metaData *head; /**< Pointer to the head of the free list */
    bool isInit;    /**< isInit signifies whether the free list has been initialized */
} freeList;

/**
 * @brief
 * @param address : It is the address of the pointer passed. It will also be the starting address of the block to be created.
 * @param size_inc_meta  : It is the size requested inculding the size of the meta data
 * @return : It returns a default block which has its size set as per requirement, and its default status is free.
 */
metaData *createBlock(void *address, int size_w_meta)
{
    if (address != NULL)
    {
        metaData *block_ptr = (metaData *)address;        /*<type casting the pointer of data type void to pointer of data type metaData, adress will now point to a variable of type metaData>*/
        block_ptr->size = size_w_meta - sizeof(metaData); /*<To avoid overlapping the space for metadata with the required size>*/
        block_ptr->isFree = FREE;                         /*<Set status to free>*/
        block_ptr->prev = NULL;                           /*<Set previous pointer to NULL>*/
        block_ptr->next = NULL;                           /*<Set next pointer to NULL>*/
        return block_ptr;
    }
    else
    {
        printf("\nInvalid address, returned NULL");
        return NULL;
    }

    // Handle case where address is NULL
}

/**
 * @brief : Function to initialize the freeList
 * @note : freeList gets initialized only once and that too implicitly. This function has been structured so as to allow encapsulation.
 */
void initialize()
{
    freeList.head = createBlock(myHeap, HEAP_SIZE);
    freeList.isInit = true;
    printf("\nfreeList.head has been initialized to: %p", freeList.head);
}

void *myMalloc(int size_req)
{
    if (!freeList.isInit)
    {
        initialize(); /*<Check if freeList has been initialized>*/
    }
    int size_w_meta = size_req + sizeof(metaData); /*<This is the size reuired to create a new block of size = requested size.>*/

    if (size_w_meta >= HEAP_SIZE) /*<We should not allocate the entire heap because of: Memory Overflow, Inefficient Memory Usage, Reduced Flexibility, Potential for Resource Exhaustion>*/
    {
        printf("\n!ERROR: Requested size exceeds heap size.");
    }
    else
    {
        /*<look for the right block>*/
        metaData *nptr = freeList.head;
        while ((nptr != NULL) && (nptr->isFree == ALLOCATED || nptr->size < size_w_meta))
        {
            nptr = nptr->next;
        }
        if (nptr == NULL)
        {
            printf("\n!ERROR: Memory allocation failed. Sufficient space not found.");
        }
        else if (nptr->isFree && nptr->size == size_w_meta) // Sizes match exactly, no need to split
        {
            if (nptr->prev == NULL)
            {
                freeList.head = nptr->next;
            }
            nptr->isFree = ALLOCATED;
            return ((void *)nptr + sizeof(metaData));
        }
        else
        {

            /*<take out required memory>*/
            metaData *fblock = createBlock((void *)nptr + sizeof(metaData) + size_req, nptr->size - size_req);
            if (nptr->prev == NULL)
            {
                freeList.head = fblock;
            }
            fblock->next = nptr->next;
            if (nptr->next != NULL)
            {
                nptr->next->prev = fblock;
            }
            nptr->next = NULL;
            fblock->prev = nptr->prev;
            if (nptr->prev != NULL)
            {
                nptr->prev->next = fblock;
            }
            nptr->prev = NULL;
            nptr->isFree = ALLOCATED;
            nptr->size = size_req;

            return ((void *)nptr + sizeof(metaData));
        }
    }
    return NULL;
}

void myFree(void *fptr)
{
    if (fptr != NULL)
    {
        metaData *ptr = (((metaData *)fptr) - 1);
        metaData *prv = NULL;
        metaData *nptr = freeList.head;
        /*<Find the block to be freed>*/
        while ((nptr != NULL) && (nptr < ptr))
        {
            prv = nptr;
            nptr = nptr->next;
        }
        if (nptr == NULL)
        {
            printf("\nFreeing %p...", fptr);
            ptr->isFree = FREE;
            prv->next = ptr;
            ptr->prev = prv;
            ptr->next = NULL;
            printf("\nFreed.");
        }
        else if (nptr == freeList.head)
        {
            printf("\nFreeing %p...", fptr);
            ptr->isFree = FREE;
            ptr->next = nptr;
            ptr->prev = NULL;
            nptr->prev = ptr;
            freeList.head = ptr;
            printf("\nFreed.");
            printf("\nfreeList.head has been changed to: %p", freeList.head);
        }
        else
        {
            printf("\nFreeing %p...", fptr);
            ptr->isFree = FREE;
            ptr->next = nptr;
            nptr->prev = ptr;
            prv->next = ptr;
            ptr->prev = prv;

            /*<If the next block is free, merge>*/
            if ((ptr->next != NULL) && (ptr->next == ((void *)ptr + sizeof(metaData) + ptr->size)) && (ptr->next->isFree == FREE))
            {
                ptr->size = ptr->size + ptr->next->size + sizeof(metaData);
                ptr->next = ptr->next->next;
                if (ptr->next != NULL)
                {
                    ptr->next->prev = ptr;
                }
                printf("\nMerged with next. nptr->size is %d", ptr->size);
            }

            /*<If the previous block is free, merge>*/
            if ((ptr->prev != NULL) && (ptr == ((void *)ptr->prev + sizeof(metaData) + ptr->prev->size)) && (ptr->prev->isFree == FREE))
            {
                ptr->prev->size = ptr->prev->size + ptr->size + sizeof(metaData); // Include sizeof(metaData)
                ptr->prev->next = ptr->next;
                if (ptr->next != NULL)
                {
                    ptr->next->prev = ptr->prev;
                }
                printf("\nMerged with previous. nptr->prev->size is %d", ptr->prev->size);
            }

            printf("\nFreed.");
        }
    }
}

void print()
{
    printf("\n\n------------------------------------ Heap contents ------------------------------------------- \n");
    printf("\nSize of metadata: %d bytes\n", sizeof(metaData));
    metaData *blockInfo = freeList.head;
    while (blockInfo != NULL)
    {
        printf("Block at Address %p:\n", (void *)blockInfo);
        printf("    Size: %d\n", blockInfo->size);
        printf("    Free: %s\n", blockInfo->isFree ? "FREE" : "ALLOCATED");
        printf("\n");
        blockInfo = blockInfo->next;
    }
    printf("-----------------------------------------------------------------------------------------------\n");
}

int main()
{
    int *cptr;
    char *dptr;
    float *fptr;
    int *cptr2;
    cptr = (int *)myMalloc(sizeof(int));
    if (cptr == NULL)
    {
        printf("\nFailed.");
    }
    else
    {
        printf("\nReceived address: %p", cptr);
    }
    print();
    dptr = (char *)myMalloc(sizeof(char) * 10);
    if (dptr == NULL)
    {
        printf("\nFailed.");
    }
    else
    {
        printf("\nReceived address: %p", dptr);
    }
    print();
    myFree(cptr);
    cptr = NULL;
    print();
    myFree(dptr);
    dptr = NULL;
    print();
    fptr = (float *)myMalloc(sizeof(float) * 2);
    if (fptr == NULL)
    {
        printf("\nFailed.");
    }
    else
    {
        printf("\nReceived address: %p", fptr);
    }
    print();
    cptr2 = (int *)myMalloc(sizeof(int));
    if (cptr2 == NULL)
    {
        printf("\nFailed.");
    }
    else
    {
        printf("\nReceived address: %p", cptr2);
    }
    print();
    myFree(fptr);
    fptr = NULL;
    print();
    myFree(cptr2);
    cptr2 = NULL;
    print();
    return 0;
}

// printf("    Content:");

        // for (size_t i = 0; i < block_info->size; i++)
        // {
        //     printf(" %02x", 0xFF & (unsigned int)(*((char *)block_info + sizeof(metaData) + i)));
        // }