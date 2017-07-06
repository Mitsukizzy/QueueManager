/*
    Sucker Punch Programming Test
    Implementation by Izzy Benavente
    July 2017
*/

#define MAX_MEMORY 2048
#define MAX_QUEUES 64

#include <stdio.h> // For printf

/*
    Memory organization in bytes
    -----------------------------------
    0               : Queue Count
    1 - 4           : Header info for q0
    5 - 8           : Header info for q1
    9 - 12          : Header info for q2
    13 - 256        : Remaining headers (up to q63)
    257 - 1948      : Data for queues
    1949 - 1981     : Data for q2
    1982 - 2014     : Data for q1
    2015 - 2047     : Data for q0
*/

unsigned char data[MAX_MEMORY]; // Container for queue memory storage

struct Q
{
    unsigned int front : 12;    // Front index of a queue 
    unsigned int back : 12;     // Back index of a queue
    unsigned int id : 6;        // Queue identifier, up to 64
    unsigned int isEmpty: 1;    // Set to 1 if empty, 0 if not
    unsigned int unused : 1;
};

// Required
Q * create_queue();
void destroy_queue(Q *q);
void enqueue_byte(Q *q, unsigned char b);
unsigned char dequeue_byte(Q *q);
// Helpers
unsigned int get_front_index();
unsigned int get_back_index();
int get_new_queue_front();
int get_buffer_space();
// Errors
void on_out_of_memory();
void on_illegal_operation();

int main()
{
    // Init the queue count
    data[0] = 0; 

    Q *q0 = create_queue();
    enqueue_byte(q0, 0);
    enqueue_byte(q0, 1);
    Q *q1 = create_queue();
    enqueue_byte(q1, 3);
    enqueue_byte(q0, 2);
    enqueue_byte(q1, 4);
    printf("%d", dequeue_byte(q0));
    printf("%d\n", dequeue_byte(q0));
    enqueue_byte(q0, 5);
    enqueue_byte(q1, 6);
    printf("%d", dequeue_byte(q0));
    printf("%d\n", dequeue_byte(q0));
    destroy_queue(q0);
    printf("%d", dequeue_byte(q1));
    printf("%d", dequeue_byte(q1));
    printf("%d\n", dequeue_byte(q1));   
    destroy_queue(q1); 
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
        newQueue->front = get_new_queue_front();
        newQueue->back = newQueue->front;
        newQueue->id = qCount;
        printf("Created queue #%d. Front index is %u and back index is %u\n", qCount, newQueue->front, newQueue->back );

        data[0] = qCount;
        return newQueue;
    }
    else
    {
        on_out_of_memory(); 
    }
}

void destroy_queue(Q *q)
{
    // Check if any queues exist
    if( data[0] <= 0 )
    {
        on_illegal_operation();
    }

    int numQueuesToShift = data[0] - q->id;
    printf("Destroying Q#%d  front: %d   back: %d\n", q->id, q->front, q->back);

    // Move all following headers left
    for(int i = 0; i < numQueuesToShift; i++)
    {
        q++;
        q->id -= 1; 
    }

    // Decrement queue count
    data[0] -= 1;
}

void enqueue_byte(Q *q, unsigned char b)
{
    int dataBack = get_back_index();
    int headerLen = 1 + 4 * data[0];
    bool midSpaceOpen = ((dataBack - headerLen) >= 1) ? true : false;

    // If q is most recent queue added (leftmost queue in storage area)
    if(q->id == data[0])
    {
        if(!midSpaceOpen)
        {
            // TODO: SLIDE ALL RIGHT
        }
    }
    else
    {        
        // Get front index of the next queue from header data
        Q* nextQ = q + 4;
        int nextQFront = nextQ->front; 
        int freeQueueSlots = q->back - nextQFront;

        if(freeQueueSlots <= 0 && !midSpaceOpen)
        {
            on_out_of_memory();
        }
        else if(freeQueueSlots <= 0 && midSpaceOpen)
        {
            // To make space, slide the data of all following queues one space left
            printf("Slide all left");
            int numQueuesToShift = data[0] - q->id;
            int shiftAmount = get_buffer_space();

            for(int i = 0; i < numQueuesToShift; i++)
            {
                for(int i = nextQ->front; i > nextQ->back; i--)
                {
                    data[i] = data[i-shiftAmount];
                    //printf("i = %d, replacing %u with %u\n", i, data[i+1], data[i]);
                }
                nextQ->front -= shiftAmount;
                nextQ->back -= shiftAmount;
                nextQ++;
            }
        }
    }
    
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
    printf("1st Enqueue: Added %u to queue #%d index %d\n", b, q->id, q->back );
}

unsigned char dequeue_byte(Q *q)
{
    // Verify that at least one queue exists
    if(data[0] <= 0)
    {
        on_illegal_operation();
    }
    else
    {
        unsigned char b = data[q->front];

        // Shift all elements in the queue one step to the front (right)
        for(int i = q->front; i > q->back; i--)
        {
            data[i] = data[i-1];
        }

        if(q->front == q->back)
        {
            q->isEmpty = 1; // Now empty after element removal
        }
        else
        {
            q->back += 1;
        }
        return b;
    }
}

// Determines the best index for a new queue to start at
int get_new_queue_front()
{    
    int qCount = data[0];

    // Give the first queue the last (rightmost) data slot
    if(qCount <= 0)
    {
        return MAX_MEMORY - 1; 
    }

    int headerLen = 1 + 4 * qCount;
    int dataBack = get_back_index();
    int midSpaceOpen = dataBack - headerLen;

    // Check if there's space for new queue 
    // Need 4 bytes for header and at least 1 byte for storage
    if(midSpaceOpen >= 5)
    {
        return dataBack - get_buffer_space();
    }
    else
    { 
        // Make space by sliding all queue storage right
        printf("Slide all right");
        Q *q = reinterpret_cast<Q*>(&data[1]);
        Q *nextQ = q + 4;

        for(int i = 0; i < qCount; i++)
        {
            int shift = q->back - nextQ->front;
            for(int i = nextQ->front; i > nextQ->back; i--)
            {
                data[i+shift] = data[i];
                //printf("i = %d, replacing %u with %u\n", i, data[i+1], data[i]);
            }
            q = nextQ;
            nextQ++;
        }
        return get_back_index() - 1;
    }
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
    int headerLen = 4 * data[0] + 1;
    int dataFront = get_front_index();
    int dataBack = get_back_index();
    int midSpaceOpen = dataBack - headerLen;
    int weightedBuffer = midSpaceOpen * ( (float)data[0] / MAX_QUEUES );
    int evenBuffer = midSpaceOpen / data[0];

    //printf("dataBack: %d   midSpaceOpen: %d    weightedBuffer: %d\n", dataBack, midSpaceOpen, weightedBuffer);

    if(weightedBuffer < evenBuffer)
    {
        return weightedBuffer;
    }
    return evenBuffer;
}

// These functions will be provided. 
void on_out_of_memory()
{
    printf("Out of memory.");
}

void on_illegal_operation()
{
    printf("Illegal operation.");
}