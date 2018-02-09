# C-Collections
A small library of C "template" data structures.

[TODO](#todo)

So this is really just a library of common data structures that i otherwise find myself rewriting all the time.
Most of the data structures are implemented with a *kind of* template-engine using macros to generate code for a specific type.
The containers also have foreach loops, they too implemented with macros.
Everything in here covers only basic functionality.

----

# Array.h
A dynamic array.
    Exposes functions:
        - init_array(Type)
            Macro to generate code for an array containing elements of type Type. E.g init_array(int) will generate the type Array_int along with functions for Array_int,
            like array_int_new(), and array_int_push()
        - array_Type_new()
            Returns empty Array of Type with initial capacity of 16 elements
        - array_Type_hash(const Array_Type*)
            Returns 32 bit hash value generated from running murmurhash on the underlying array of elements
        - array_Type_free(Array_Type*)
            Frees underlying array, and sets everything to 0
        - array_Type_copy(const Array_Type*)
            Returns deep copy of the Array passed as argument
        - array_Type_expand(Array_Type*)
            Doubles the capacity of the array. Really shouldn't be used since the array handles this automatically, but it's there...
        - array_Type_push(Array_Type*, const Type)
            Pushes item to the back of the Array, expands if neccessary
        - array_foreach(Type, array, lambda)
            Macro that loops through all elements in array and calls lambda, passing them as argument.
            lambda should be a function or macro taking a single parameter of type Type.
            array should be the Array_Type to iterate over.
            Type is not actually used, but included for consistency with the other data types
    
# TODO
  - Double Linked List
  - Priority Queue
  - HashMap
  - HashSet
  - Some kind of Tree Map
  - Some kind of Tree Set
