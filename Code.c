#include<stdio.h>
#include<stdlib.h>
#define MAX_BLOCK_SIZE 10000
#define THRESHOLD 100 // if (block_size - process_size ) < threshold => allocate entire block
typedef enum{FALSE,TRUE} bool;

typedef struct meta_data_node
{
    int block_size;
    bool is_allocated; // bool to check whether the current block is allocated or not 
    struct meta_data_node* next_free; // To maintain the free list using pointer
    struct meta_data_node* prev_free; // Doubly linked list to maintain free list
    struct meta_data_node* next; //pointer which helps in traversing each and every block (be it free or allocated)
    struct meta_data_node* prev; //pointer which helps in traversing each and every block (be it free or allocated)

}meta_data;

meta_data* head ; // global variable to maintain the meta data doubly linked list

void intialize(meta_data* ptr)
{
	if(ptr == head)
	{
		ptr->block_size = MAX_BLOCK_SIZE;
	}
	else
	{
		ptr->block_size = 0;
	}    
    ptr->is_allocated = FALSE;
    ptr->next=NULL;
    ptr->prev=NULL;
    ptr->next_free=NULL;
    ptr->prev_free=NULL;
}

meta_data* make_free_meta_data_node(meta_data* ptr,int size)
{
    meta_data* temp;
    temp = (meta_data* ) malloc(sizeof(meta_data));

    temp->block_size = size;//diff
    temp->is_allocated = FALSE;
    temp->next = ptr->next;
    temp->prev = ptr;
    temp->next_free = ptr->next_free;
    temp->prev_free = ptr->prev_free;
    return temp;
}

meta_data* allocate(int process_size) // allocate function which uses first fit allocation strategy with an explicit list of free blocks to allocate.
{
    meta_data* ptr = NULL ; // traversing pointer
    meta_data* temp; 
    
    bool found = FALSE;
    int diff;
    if(head->is_allocated == FALSE)
    {
        ptr = head;
    }
    else
    {
        ptr = head->next_free;
    }

    while ((ptr != NULL) && (found == FALSE))
    {
        if ((ptr->block_size >= process_size) && (ptr->is_allocated == FALSE))
        {
            found = TRUE;
            diff = (ptr->block_size - process_size);
            if(diff >= THRESHOLD)
            {
                ptr->block_size = process_size;
                ptr->is_allocated = TRUE;
                temp = make_free_meta_data_node(ptr,diff);

                if (ptr->prev_free != NULL) // edge case
                {
                    (ptr->prev_free)->next_free = temp; // now pointing to the newer free block 
                }
                meta_data* iptr = ptr->prev; // iptr => iterating ptr
                while(iptr != ptr->prev_free) 
                {
                    (iptr)->next_free = temp;
                    iptr = iptr->prev;
                }
                iptr = NULL;
                if (ptr->next_free != NULL) // edge case
                {
                    (ptr->next_free)->prev_free = temp; // now pointing to the newer free block
                }
                iptr = ptr->next;
                while (iptr != ptr->next_free)
                {
                    iptr->prev_free = temp;
                    iptr = iptr->next;
                }
                iptr=NULL;

                ptr->next_free = temp;
                ptr->next = temp; // as temp has emerged from ptr so it will be very next to it .                        
            }
            else // No free block created out of existing
            {
                // Don't make any changes to block size as
                // here we are giving the entire block to the process.

                ptr->is_allocated = TRUE;
                
                if (ptr->prev_free != NULL) // edge case
                {
                    (ptr->prev_free)->next_free = ptr->next_free; // now pointing to the next free block of the current block
                }
                meta_data* iptr = ptr->prev; // iptr => iterating ptr
                while(iptr != ptr->prev_free) 
                {
                    (iptr)->next_free = ptr->next_free;
                    iptr = iptr->prev;
                }
                iptr = NULL;

                if (ptr->next_free != NULL) // edge case
                {
                    (ptr->next_free)->prev_free = ptr->prev_free; // now pointing to the newer free block
                }
                iptr = ptr->next;
                while (iptr != ptr->next_free)
                {
                    iptr->prev_free = ptr->prev_free;
                    iptr = iptr->next;
                }
                iptr=NULL;
                
                // ptr->next_free and ptr->next are unchanged because there isn't any creation of a new node here. 
            }
        }
        else
        {
            ptr = ptr->next_free;
        }                
    }
    if(found == TRUE)
    {
        printf("Process was allocated desired size\n");
        return ptr;
    } 
    else
    {
        printf("Process Size too large to be allocated\n");
        return NULL;
    }
}

// assuming the ptr passed is valid that is such a pointer exists in the meta data list 
void free_ptr(meta_data* ptr) // ptr pointing to memory which is supposed to freed
{  
    if(ptr==NULL)
    {
        return;
    }
    meta_data* temp_for_free; // for freeing the not in use meta data ptrs    
    ptr->is_allocated = FALSE; // Since it must have been initially set to true
    if (ptr->prev_free != NULL) // edge case
    {
        (ptr->prev_free)->next_free = ptr; // now pointing to the currently freed block
    }
    meta_data* iptr = ptr->prev; // iptr => iterating ptr
    while(iptr != ptr->prev_free) 
    {
        (iptr)->next_free = ptr;
        iptr = iptr->prev;
    }
    iptr=NULL;
    if (ptr->next_free != NULL) // edge case
    {
        (ptr->next_free)->prev_free = ptr; // now pointing to the currently freed block
    }
    iptr = ptr->next;
    while (iptr != ptr->next_free)
    {
        iptr->prev_free = ptr;
        iptr = iptr->next;
    }
    iptr=NULL;

    // Merging Part 
    
    /* ALGO */
    /* 1) if prev = NULL nd next != NULL then check only for next , if adjacent or not , if yes , then merge next into ptr , and free(actually) next
       2) if next = NULL nd prev != NULL then check only for prev , if adjacent or not , if yes , then merge prev into ptr , but if prev = head then also move head to ptr , then finally free(actually) prev
       3) if both are not equal to NULL then write a code similar to one which is below for merging ,but change some conditions, here also if prev is head then bring head to ptr 
       4) if both equal to null then its our initial state 
    */
    if ((ptr->prev_free == NULL) && ((ptr->next_free) != NULL)) 
    {
        
        if(ptr->next_free == ptr->next)// =>adjacent
        {
            ptr->block_size += (ptr->next_free)->block_size;
            ptr->next = (ptr->next)->next;
            if(ptr->next_free->next != NULL)
            {
                ptr->next_free->next->prev = ptr;
            }
            temp_for_free = ptr->next_free;
            ptr->next_free = (ptr->next_free)->next_free; 
            if((ptr->next_free) != NULL)
            {
                ptr->next_free->prev_free = ptr;
            }
            iptr = ptr->next; // iptr => iterating ptr
            while(iptr != ptr->next_free) 
            {
                (iptr)->prev_free = ptr;
                iptr = iptr->next;
            }
            iptr=NULL;
            intialize(temp_for_free);
            free(temp_for_free);//actually freeing the meta data 
            temp_for_free = NULL;
        }
    }
    if((ptr->prev_free != NULL) && (ptr->next_free) == NULL)
    {
        if ((ptr->prev_free) == ptr->prev)// adjacent
        {
            ptr->block_size += (ptr->prev_free)->block_size;

            if(ptr->prev_free == head)
            {
                head = head->next; // that is head = ptr
            }

            if(ptr->prev_free->prev != NULL)
            {
                ptr->prev_free->prev->next = ptr;
            }
            temp_for_free = ptr->prev_free;
            ptr->prev_free = ptr->prev_free->prev_free; // ptr->prev_free updated

            iptr = ptr->prev; // iptr => iterating ptr
            while(iptr != ptr->prev_free) 
            {
                (iptr)->next_free = ptr;
                iptr = iptr->prev;
            }
            iptr=NULL;
            
            intialize(temp_for_free);
            free(temp_for_free);// freeing the meta data node which is not in use now after merging
            temp_for_free = NULL;
        }                
    }
    if((ptr->prev_free != NULL) && (ptr->next_free) != NULL)
    {
        if(ptr->next_free == ptr->next)// =>adjacent
        {
            ptr->block_size += (ptr->next_free)->block_size;
            ptr->next = (ptr->next)->next;
            if(ptr->next_free->next != NULL)
            {
                ptr->next_free->next->prev = ptr;
            }
            temp_for_free = ptr->next_free;
            ptr->next_free = (ptr->next_free)->next_free; 
            if((ptr->next_free) != NULL)
            {
                ptr->next_free->prev_free = ptr;
            }
            iptr = ptr->next; // iptr => iterating ptr
            while(iptr != ptr->next_free) 
            {
                (iptr)->prev_free = ptr;
                iptr = iptr->next;
            }
            iptr=NULL;
            intialize(temp_for_free);
            free(temp_for_free);//actually freeing the meta data 
            temp_for_free = NULL;
        }
        // if right side free block was also adjacent then ptr must have been updated by now 
        if ((ptr->prev_free) == ptr->prev)// adjacent
        {
            ptr->block_size += (ptr->prev_free)->block_size;
            ptr->prev = (ptr->prev)->prev;
            if(ptr->prev_free == head)
            {
                head = head->next; // that is head = ptr
            }

            if(ptr->prev_free->prev != NULL)
            {
                ptr->prev_free->prev->next = ptr;
            }
            temp_for_free = ptr->prev_free;
            ptr->prev_free = ptr->prev_free->prev_free; // ptr->prev_free updated

            iptr = ptr->prev; // iptr => iterating ptr
            while(iptr != ptr->prev_free) 
            {
                (iptr)->next_free = ptr;
                iptr = iptr->prev;
            }
            iptr=NULL;
            
            intialize(temp_for_free);
            free(temp_for_free);// freeing the meta data node which is not in use now after merging
            temp_for_free = NULL;
        } 
    }
    // if(ptr->prev_free == ptr->next_free == NULL) => No merging required
}

// void display_detail(meta_data* head)
// {
//     meta_data* ptr;
//     ptr = head;
//     while (ptr != NULL)
//     {
//         printf("The current block size is %d ",ptr->block_size);
//         if(ptr->is_allocated == TRUE)
//         {
//             printf("and is allocated\n");
//         }
//         else
//         {
//             printf("and is free\n");
//         }
//         if(ptr->prev == NULL)
//         {
//             printf("The prev pointer value is NULL \n");
//         }
//         else
//         {
//             printf("The prev pointer's block size is %d\n",ptr->prev->block_size);
//         }
//         if(ptr->prev_free == NULL)
//         {
//             printf("The prev free pointer value is NULL \n");
//         }
//         else
//         {
//             printf("The prev free pointer's block size is %d\n",ptr->prev_free->block_size);
//         }
//         if(ptr->next == NULL)
//         {
//             printf("The next pointer value is NULL \n");
//         }
//         else
//         {
//             printf("The next pointer's block size is %d\n",ptr->next->block_size);
//         }
//         if(ptr->next_free == NULL)
//         {
//             printf("The next free pointer value is NULL \n");
//         } 
//         else
//         {
//             printf("The next free pointer's block size is %d\n",ptr->next_free->block_size);
//         }

//         printf("\n");
//         ptr = ptr->next;
//     }    
// }


void display(meta_data* head)
{
    printf("\n");  
    printf("\n");
    meta_data* ptr;
    ptr = head;
    while (ptr != NULL)
    {
        printf("%d ",ptr->block_size);
        if(ptr->is_allocated == TRUE)
        {
            if(ptr->next == NULL)
            {
                printf("is allocated");
            }
            else
            {
                printf("is allocated <-> ");
            }            
        }
        else
        {
            if(ptr->next == NULL)
            {
                printf("is free");
            }
            else
            {
                printf("is free <-> ");
            }
        }
        ptr = ptr->next;
    } 
    printf("\n");
    printf("\n");     
}

int main()
{
    printf("It is assumed that the initial block size (which is free initially) is 10000 and the threshold value is 100\n");
    head = (meta_data*) malloc(sizeof(meta_data)); // Global meta_data_list
    intialize(head);
    int n ;
    printf("Enter the number of processes: \n ");
    scanf("%d",&n);
    int process[n];
    int i;
    printf("Enter the process sizes: \n ");
    for ( i = 0; i < n; i++)
    {
        scanf("%d",&process[i]);
    }
    printf("\n");
    meta_data* ptr[n];
    for ( i = 0; i < n; i++)
    {
        ptr[i] = allocate(process[i]);
    }
    display(head);
	printf("Enter the number of blocks which u wish to free\n");
    scanf("%d",&n);
    printf("\n");
    printf("Enter the indices of block pointers which u wish to free\n");
    int arr[n];
    for ( i = 0; i < n; i++)
    {
        scanf("%d",&arr[i]);
    }    
    for ( i = 0; i < n; i++)
    {
        free_ptr(ptr[arr[i]]);
        ptr[arr[i]]=NULL;
        display(head);
    }

    printf("Enter the number of processes you want to allocate now\n");
    scanf("%d",&n);
    printf("Enter process sizes\n");
    for ( i = 0; i < n; i++)
    {
        scanf("%d",&process[i]);
    }
    meta_data* new_ptr[n];
    printf("\n");
    for (i = 0; i < n; i++)
    {
        allocate(process[i]);
    }// enough for demonstration purpose
    display(head);
    printf("\nEnd of demonstration\n");  
    
    return 0;
}
