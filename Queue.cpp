#define MAX_MEMORY 2048
#define MAX_QUEUES 64

#include <stdio.h> // For printf


unsigned char data[MAX_MEMORY]; // Container for queue memory storage

struct Q
{
    unsigned int isEmpty: 1;    // Set to 1 if empty, 0 if not
    unsigned int front : 11;    // Able to store values up to 2048 (our array indices)
    unsigned int back : 11;
    unsigned int qID : 6;       // Queue identifier, up to 64
};

Q * create_queue();
void destroy_queue(Q *q);
void enqueue_byte(Q *q, unsigned char b);
unsigned char dequeue_byte(Q *q);
unsigned int get_front_index();
unsigned int get_back_index();
int get_next_free_slot();
int get_buffer_space();
void on_out_of_memory();
void on_illegal_operation();

int main()
{
    printf("You can do this! \n" );
    data[0] = 0; // Reserve the first data slot to maintain queue count
    Q *q0 = create_queue();
    enqueue_byte(q0, 0);
    enqueue_byte(q0, 1);
    Q *q1 = create_queue();
    enqueue_byte(q1, 3);
    enqueue_byte(q0, 2);
    enqueue_byte(q1, 4);
    printf("%d", dequeue_byte(q0));
    printf("%d\n", dequeue_byte(q0));
    // enqueue_byte(q0, 5);
    // enqueue_byte(q1, 6);
    // printf("%d", dequeue_byte(q0));
    // printf("%d\n", dequeue_byte(q0));
    // destroy_queue(q0);
    // printf("%d", dequeue_byte(q1));
    // printf("%d", dequeue_byte(q1));
    // printf("%d\n", dequeue_byte(q1));   
    // destroy_queue(q1); 
}

Q * create_queue()
{
    int qCount = data[0];

    // Check if there is space for a new queue
    if( qCount < MAX_QUEUES )
    {
        Q *newQueue = reinterpret_cast<Q*>(&data[1 + 4 * qCount]);
        qCount++;
        newQueue->isEmpty = 1;
        newQueue->front = get_next_free_slot();
        newQueue->back = newQueue->front;
        newQueue->qID = qCount;
        printf("Created queue #%d. Front index is %u and back index is %u\n", qCount, newQueue->front, newQueue->back );

        data[0] = qCount;

        return newQueue;
    }
    else
    {
        on_out_of_memory(); 
    }
}

// void destroy_queue(Q *q)
// {
//     // Check if any queues exist
//     if( data[0] <= 0 )
//     {
//         on_illegal_operation();
//     }

//     // Move all headers one left

//     // Decrement queue count
//     data[0] = data[0] - 1;
// }

void enqueue_byte(Q *q, unsigned char b)
{
    int nextFreeSlot = get_next_free_slot();
    //printf("Q#%d\n", q->qID);

    // Most recent queue added and space free
    if(q->qID == data[0] && nextFreeSlot >= 1)
    {        
        if(q->isEmpty)
        {
            q->isEmpty = 0; // No longer empty
            data[q->back] = b;
        }
        else
        {
            q->back -= 1;   
            data[q->back] = b;
        }
        printf("1st Enqueue: Added %u to queue #%d index %d\n", b, q->qID, q->back );
    }
    else
    {        
        // Get front index of the next queue from header data
        Q* nextQ = q + 4;
        int nextQFront = nextQ->front; 
        int freeQueueSlots = q->back - nextQFront;

        if(freeQueueSlots >= 1)
        {            
            q->back -= 1;
            data[q->back] = b;
            printf("2nd Enqueue: Added %u to queue #%d index %d\n", b, q->qID, q->back );
        }

        // Space available outside of this queue
        if(nextFreeSlot >= 1)
        {
            // TODO: Slide all following queues left
        }
        else
        {
            on_out_of_memory();
        }
    }
}

unsigned char dequeue_byte(Q *q)
{
    // Check if any queues exist
    if( data[0] <= 0 )
    {
        on_illegal_operation();
    }
    else
    {
        unsigned char b = data[q->front];
        printf("Removing %u from queue #%d back index %d\n", b, q->qID, q->back);

        // Shift all elements in the queue one step to the front (right)
        for(int i = q->front; i > q->back; i--)
        {
            data[i] = data[i-1];
            printf("i = %d, replacing %u with %u\n", i, data[i+1], data[i]);
        }
        q->back += 1;
        return b;
    }
}

// Helper Functions

// Returns the next free slot
int get_next_free_slot()
{    
    int qCount = data[0];

    if(qCount <= 0)
    {
        // Only the first data slot is in use
        return MAX_MEMORY - 1; // Give the rightmost data slot
    }

    int headerLen = 1 + 4 * qCount;
    int dataBack = get_back_index();
    int totalFree = dataBack - headerLen;

    Q *q = reinterpret_cast<Q*>(&data[headerLen - 4]);
    printf("GNFS - qID: %u     back: %u     front: %u     q: %u\n", q->qID, q->back, q->front, q );

    if(totalFree <= 0)
    {
        return -1;
    }

    int buffer = get_buffer_space();
    printf("Buffer: %d\n", buffer);
    if(totalFree > buffer)
    {
        return dataBack - buffer;
    }
    return dataBack;
}

unsigned int get_front_index()
{    
    Q *firstQ = reinterpret_cast<Q*>(&data[1]);
    return firstQ->front;
}

unsigned int get_back_index()
{    
    int headerLen = 4 * data[0] + 1;
    Q *lastQ =  reinterpret_cast<Q*>(&data[headerLen - 4]);
    return lastQ->back;
}

// Returns the average queue length to give most recent queue some buffer space
int get_buffer_space()
{
    int dataFront = get_front_index();
    int dataBack = get_back_index();
    int nonHeaderSpaceUsed = dataFront - dataBack;
    int equalSpace = MAX_MEMORY / MAX_QUEUES;
    printf("dataBack: %d   nonHeaderSpaceUsed: %d\n", dataBack, nonHeaderSpaceUsed);

    if(nonHeaderSpaceUsed < equalSpace)
    {
        return equalSpace;
    }

    return nonHeaderSpaceUsed / data[0];
}


void on_out_of_memory()
{
    printf("Out of memory.");
}

void on_illegal_operation()
{
    printf("Illegal operation.");
}