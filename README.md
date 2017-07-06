# QueueManager

### The Task:
## Create a manager with a set of functions that can handle a variable number of byte queues, which each have a variable length, in a small, fixed amount of memory.

All storage other than local variables in function must be in the following array.
unsigned char data[2048]

The code cannot call malloc or other heap management routines. 

No more than 64 queues will be created at once.



### My Approach
Given the small amount of space, I thought about how to best keep track of the queues while utilizing the Q type, which could be whatever I wanted. I refer to the information in the Q struct as header info, which includes front index, back index, an isEmpty flag, and the ID of the queue. I store this header info in ascending order, starting from the start (left side) of the data array. On the other hand, the stored bytes which the queues contain are positioned at the end (right side) of the array.

When a new queue is added, I add some extra space between its front and the preceding queue's back. Destroying a queue is relatively simple, as I take the header information of all queues succeeding q and slide them over to the left, each of them becoming queue #(i-1), keeping the left side of the array always sequential and contiguous.