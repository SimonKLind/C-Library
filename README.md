# C-Collections
A small library of C "template" data structures.

So this is really just a library of common data structures that i otherwise find myself rewriting all the time.
Most of the data structures are implemented with a *kind of* template-engine using macros to generate code for a specific type.
The containers also have foreach loops, they too implemented with macros.
Everything in here covers only basic functionality.

As of this writing there are the following pieces in this library:

  - Hash.h 
    - A small collection of hash functions, including implementations of fnv-1a for both 32 and 64 bits, and murmurhash3 for 32 bits.
    
  - String.h
    - A String container with short-string optimization, always null-terminated
    
  - Array.h
    - A dynamic Array
    
  - Stack.h
    - A Last-In-First-Out stack
    
   - LinkedList.h
      - A First-In-First-Out Linked List
    
# Example Usage
To generate code for e.g an array to hold double
  - init_array(double)
Note: no semicolon needed

To use the foreach loops, again example array of type double
  - #define lambda(arg) // whatever you want your lambda to do with arg
  - array_foreach(double, array, lambda)
  - Optionally #undef lambda
    
# TODO
  - Double Linked List
  - Priority Queue
  - HashMap
  - HashSet
  - Some kind of Tree Map
  - Some kind of Tree Set
