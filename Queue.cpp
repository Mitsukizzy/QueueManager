#define MAX_MEMORY 2048
#define MAX_QUEUES 64

#include <stdio.h> // For printf


unsigned char data[MAX_MEMORY]; // Container for queue memory storage

struct Q
{
    unsigned int front : 12;
    unsigned int back : 12;
    unsigned int qNum : 6;
};

Q * create_queue();
void destroy_queue(Q *q);
void enqueue_byte(Q *q, unsigned char b);
unsigned char dequeue_byte(Q *q);
unsigned int get_front_index();
unsigned int get_back_index();
int get_next_free_slot();
// int get_int_from_chars(unsigned char high_index);
// void save_header_info(int front, int back, int qNum);
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
    printf("Q#%d\n", q0->qNum);
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
        newQueue->front = get_next_free_slot();
        newQueue->back = newQueue->front;
        newQueue->qNum = qCount;
        printf("Created queue #%d. Front index is %u and back index is %u\n", qCount, newQueue->front, newQueue->back );

        // store Q in data
        //save_header_info(newQueue->front, newQueue->back, newQueue->qNum);
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
    //printf("Q#%d\n", q->qNum);

    // Most recent queue added and space free
    if(q->qNum == data[0] && nextFreeSlot >= 1)
    {
        data[q->back] = b;
        printf("1st Enqueue: Added %u to queue #%d index %d\n", b, q->qNum, q->back );
        q->back -= 1;

        // Save to header data
        //save_header_info(q->front, q->back, q->qNum);
    }
    else
    {        
        // Get front index of the next queue from header data
        Q* nextQ = q + 4;
        int nextQFront = nextQ->front; 
        int freeQueueSlots = q->back - nextQFront;

        if(freeQueueSlots >= 1)
        {            
            data[q->back] = b;
            printf("2nd Enqueue: Added %u to queue #%d index %d\n", b, q->qNum, q->back );
            q->back -= 1;

            // Save to header data
            //save_header_info(q->front, q->back, q->qNum);
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
        printf("Removing %u from queue #%d index %d\n", b, q->qNum, q->front);

        // Shift all elements in the queue one step to the front (right)
        for(int i = q->back; i < q->front; i++)
        {
            data[i+1] = data[i];
        }
        q->back += 1;

        // Save to header data
       // save_header_info(q->front, q->back, q->qNum);
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
    //int dataBack = get_int_from_chars(headerLen - 2);
    int dataBack = get_back_index();
    int totalFree = dataBack - headerLen;

    Q *q = reinterpret_cast<Q*>(&data[headerLen - 4]);
    printf("GNFS - qNum: %u     back: %u     front: %u     q: %u\n", q->qNum, q->back, q->front, q );
    //printf("GNFS - dataBack: %d     total free: %d\n", dataBack, totalFree );

    if(totalFree <= 0)
    {
        return -1;
    }

    int buffer = get_buffer_space();
    //printf("Buffer: %d\n", buffer);
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
    // int dataFront = get_int_from_chars(1);
    // int dataBack = get_int_from_chars(headerLen - 2);
    int nonHeaderSpaceUsed = dataFront - dataBack;
    printf("dataBack: %d   nonHeaderSpaceUsed: %d\n", dataBack, nonHeaderSpaceUsed);
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
        
// int get_int_from_chars(unsigned char low_index)
// {
//     int high_index = low_index + 1;
//     int num = (data[high_index] << 8) + data[low_index];
//     //printf("GET - INDEX: %d     NUM: %d    dlow: %u     dhigh: %u \n", low_index, num, data[low_index], data[high_index] );
//     return num;
// }

// void set_chars_from_int(int num, int header_index)
// {
//     data[header_index] = num & 0xFF; // low: masking for the least significant byte
//     data[header_index + 1] = (num >> 8) & 0xFF; // high
//     //printf("SET - INDEX: %d     NUM: %d    LOW: %u      HIGH: %u   \n", header_index, num, data[header_index], data[header_index + 1]);
// }

// void save_header_info(int front, int back, int qNum)
// {
//     int iFront = 1 + 4 * (qNum - 1);
//     int iBack = iFront + 2;
//     set_chars_from_int(front, iFront);
//     set_chars_from_int(back, iBack);
// }