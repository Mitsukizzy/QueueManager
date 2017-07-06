# QueueManager

### The Task:
### Create a manager with a set of functions that can handle a variable number of byte queues, which each have a variable length, in a small, fixed amount of memory.

All storage other than local variables in function must be in the following array.
```unsigned char data[2048]```

The code cannot call malloc or other heap management routines. No more than 64 queues will be created at once.



### My Approach
Given the small amount of space, I thought about how to best keep track of the queues while utilizing the Q type, which could be whatever I wanted. I refer to the information in the Q struct as header info, which includes things such as the front and back index of the queue it refers to. I store this header info in ascending order, starting from the start (left side) of the data array. Everything bookkeeping related, such as the headers, total queue count, and free space count, is kept on the lower end of the data array. The lower end is always kept sequential and contiguous with no gaps. On the other hand, the stored bytes which the queues track and manage are positioned at the end (right side) of the array. 

I considered implementing the queues as circular linked lists but dropped the idea since we can't assume the maximum size of any given queue other than the total memory available. Another strategy I considered was implementing the queues as linked lists with their header info at the front of each list. That implementation seemed manageable, but expensive as it would require many loops or iterations to search for a given queue. Ultimately, I liked the idea of keeping the organizational side and the storage side on separate ends of the array as it made it easier to manage the queues knowing exactly how much space they could use. The downside to my approach is the need for shifting/defragmentation, especially during an enqueue when two queues are directly next to each other. I mitigate this by keeping reasonable buffer space between queues, calculated as a function of the space available and the queues in use.

    Memory organization in bytes
    ----------------------------------------------------
    0               : Queue Count
    1 - 2           : Space Free Count
    3 - 6           : Header info for q0
    7 - 10          : Header info for q1
    11 - 14         : Header info for q2
    15 - 258        : Remaining headers (up to q63)
    259 - 2047      : Data for queues (storage area)
    ----------------------------------------------------
    Storage Area is right aligned, example below
    ----------------------------------------------------
    1949 - 1981     : Data for q2
    1982 - 2014     : Data for q1
    2015 - 2047     : Data for q0