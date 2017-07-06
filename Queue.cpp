/*
    Sucker Punch Programming Test
    Implementation by Izzy Benavente
    July 2017
*/

#define MAX_MEMORY 2048
#define MAX_QUEUES 64

#include <stdio.h> // For printf
#include <cstdlib> // For exit

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
void shift_right();
void shift_left(int qID);
void update_space_free(int modifier);
void set_space_free(unsigned int spaceFree);
unsigned int get_space_free();
unsigned int get_front_index();
unsigned int get_back_index();
int get_new_queue_front();
int get_buffer_space();
int get_header_len();
// Errors
void on_out_of_memory();
void on_illegal_operation();

int main()
{
    // Init the queue count
    data[0] = 0; 
    // Init free spaces count, stored in data[1] and data[2]
    set_space_free(MAX_MEMORY - 1);

    Q *q0 = create_queue();
    enqueue_byte(q0, 0);
    enqueue_byte(q0, 1);
    Q *q1 = create_queue();
    enqueue_byte(q1, 3);
    for( int i = 0; i < 500; i++)
    {
        enqueue_byte(q1, 21);
    }
    for( int i = 0; i < 400; i++)
    {
        enqueue_byte(q0, 70);
    }
    Q *q2 = create_queue();
    for( int i = 0; i < 800; i++)
    {
        enqueue_byte(q1, 22);
    }
    for( int i = 0; i < 800; i++)
    {
        enqueue_byte(q2, 90);
    }
    for( int i = 0; i < 700; i++)
    {
        enqueue_byte(q0, 71);
    }
    enqueue_byte(q1, 4);
    printf("%d", dequeue_byte(q0));
    printf("%d\n", dequeue_byte(q0));
    enqueue_byte(q0, 5);
    enqueue_byte(q1, 6);
    printf("%d", dequeue_byte(q0));
    printf("%d\n", dequeue_byte(q0)); 
    destroy_queue(q0); 
    destroy_queue(q2);
    printf("%d", dequeue_byte(q1));
    printf("%d", dequeue_byte(q1));
    printf("%d\n", dequeue_byte(q1));   
    destroy_queue(q1); 


    // Q *q0 = create_queue();
    // enqueue_byte(q0, 0);
    // enqueue_byte(q0, 1);
    // Q *q1 = create_queue();
    // enqueue_byte(q1, 3);
    // enqueue_byte(q0, 2);
    // enqueue_byte(q1, 4);
    // printf("%d", dequeue_byte(q0));
    // printf("%d\n", dequeue_byte(q0));
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
        Q *newQueue = reinterpret_cast<Q*>(&data[3 + 4 * qCount]);
        qCount++;
        newQueue->isEmpty = 1;
        newQueue->front = get_new_queue_front();
        newQueue->back = newQueue->front;
        newQueue->id = qCount;
        printf("Created queue #%d. Front index is %u and back index is %u\n", qCount, newQueue->front, newQueue->back );

        // Reserved 5 bytes: 4 bytes for header + 1 byte for storage 
        update_space_free(5); 
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

    // Give freed space back to the global count
    int freedSpace = (q->front - q->back) + 5;
    update_space_free(-freedSpace); 

    // Decrement queue count
    data[0] -= 1;
}

void enqueue_byte(Q *q, unsigned char b)
{
    // Check free space global count
    if(get_space_free() <= 0)
    {
        on_out_of_memory();
    }

    int dataBack = get_back_index();
    bool midSpaceOpen = ((dataBack - get_header_len()) >= 1) ? true : false;

    // If q is most recent queue added (leftmost queue in storage area)
    if(q->id == data[0])
    {
        if(!midSpaceOpen)
        {
            shift_right();
        }
    }
    else
    {        
        // Get front index of the next queue from header data
        Q* nextQ = q;
        nextQ++;
        int nextQFront = nextQ->front; 
        int freeQueueSlots = q->back - nextQFront - 1;
        printf("qID: %d,  nextQID: %d,   q->back: %d, nextQFront: %d,  FreeQueueSlots: %d\n", q->id, nextQ->id, q->back, nextQFront, freeQueueSlots);

        if(freeQueueSlots <= 0 && !midSpaceOpen)
        {
            shift_right();
        }
        else if(freeQueueSlots <= 0 && midSpaceOpen)
        {
            shift_left(q->id);
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
    update_space_free(1); 
    printf("Enqueue: Added %u to queue #%d index %d   FREESPACE: %d\n", b, q->id, q->back, get_space_free() );
}

unsigned char dequeue_byte(Q *q)
{
    // Check that there is data to remove
    if(data[0] <= 0 || q->isEmpty)
    {
        on_illegal_operation();
    }
    else
    {
        unsigned char b = data[q->front];

        // Shift all elements in the queue one step to the front (right)
        for(int i = q->front; i >= q->back; i--)
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

        update_space_free(-1); 
        return b;
    }
}

// Called when there is Limited space left in middle area
// Defragments storage area, right aligning all queue data in the array
void shift_right()
{
    Q* prevQ = reinterpret_cast<Q*>(&data[3]); // the first queue
    Q* curQ = prevQ + 4;
    int numQueuesToShift = data[0] - 1;
    int shiftAmount;
    printf("Slide all right. NumQToShift: %d. ShiftAmt: %d\n", numQueuesToShift, shiftAmount);
    
    for(int i = 0; i < numQueuesToShift; i++)
    {
        shiftAmount = prevQ->back - curQ->front; // the gap between the two queues
        for(int i = curQ->front; i >= curQ->back; i--)
        {
            printf("Qid = %d, front %u. back %u\n", curQ->id, curQ->front, curQ->back);
            printf("i = %d, replacing %u with %u\n", i, data[i+1], data[i]);
            data[i+shiftAmount] = data[i];
        }
        prevQ = curQ;
        curQ->front += shiftAmount;
        curQ->back += shiftAmount;
        curQ++;
    }
}

// To utilize space available in the middle area, shift queues left to give the current queue more room
// Only shifts queues following the current one
void shift_left(int qID)
{
    Q* curQ = reinterpret_cast<Q*>(&data[get_header_len()-4]); // the newest queue
    int numQueuesToShift = data[0] - qID;
    int shiftAmount = get_buffer_space();
    printf("Slide all left. NumQToShift: %d. ShiftAmt: %d\n", numQueuesToShift, shiftAmount);

    for(int i = 0; i < numQueuesToShift; i++)
    {
        for(int i = curQ->back; i <= curQ->front; i++)
        {
            printf("Qid = %d, front %u. back %u\n", curQ->id, curQ->front, curQ->back);
            printf("i = %d, replacing %u with %u\n", i, data[i-shiftAmount], data[i]);
            data[i-shiftAmount] = data[i];
        }
        curQ->front -= shiftAmount;
        curQ->back -= shiftAmount;
        curQ--;
    }
}

void update_space_free(int modifier)
{
    int spaceFree = get_space_free();
    spaceFree -= modifier;
    set_space_free(spaceFree);
}

void set_space_free(unsigned int spaceFree)
{    
    data[1] = (spaceFree >> 8) & 0xFF;    // high
    data[2] = spaceFree & 0xFF;           // low: masking for the least significant byte
}

unsigned int get_space_free()
{
    return (data[1] << 8) + data[2];
}

unsigned int get_front_index()
{    
    Q *firstQ = reinterpret_cast<Q*>(&data[1]);
    return firstQ->front;
}

unsigned int get_back_index()
{    
    Q *lastQ =  reinterpret_cast<Q*>(&data[get_header_len() - 4]);
    return lastQ->back;
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

    int dataBack = get_back_index();
    int midSpaceOpen = dataBack - get_header_len();

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
        Q *nextQ = q;
        nextQ++;

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

// Returns the average queue length to give most recent queue some buffer space
int get_buffer_space()
{
    int dataFront = get_front_index();
    int dataBack = get_back_index();
    int midSpaceOpen = dataBack - get_header_len();
    int weightedBuffer = midSpaceOpen * ( (float)data[0] / MAX_QUEUES );
    int evenBuffer = midSpaceOpen / data[0];

    //printf("dataBack: %d   midSpaceOpen: %d    weightedBuffer: %d\n", dataBack, midSpaceOpen, weightedBuffer);

    if(weightedBuffer < evenBuffer)
    {
        return weightedBuffer;
    }
    return evenBuffer;
}

int get_header_len()
{
    return 4 * data[0] + 3;
}

// These functions will be provided. 
void on_out_of_memory()
{
    printf("Out of memory.");
    exit(1);
}

void on_illegal_operation()
{
    printf("Illegal operation.");
    exit(1);
}