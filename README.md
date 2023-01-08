# Flexer - Growing container for data

Flexer is linear container for storing data. The storage is
continous. Storage grows when added data does not fit to the current
allocation. Reservation is doubled at resizing.

Flexer struct:

    Field     Type          Addr
    -----------------------------
    size      (uint64_t)  | N + 0
    used      (uint64_t)  | N + 8
    data      (void*)     | N + 16

`size` gives the number of bytes in the storage allocation. `used`
defines the amount of bytes stored, so far.

Base type for Flexer is `fl_t`, i.e. the Flexer. `fl_t` is a pointer
to Flexer struct (descriptor). None of the functions in Flexer library
require a reference to Flexer itself, since `fl_t` members are only
updated.

Flexer might point to a stack or a heap allocated Flexer descriptor,
depending on the use case.

Flexer can be created with default size using `fl_new()`. Default size
is 16. If user knows about usage pattern for Flexer, a specific size
can be requested using `fl_new_sized()` function.

    fl_t fl = fl_new_sized( NULL, 128 );

Flexer can be destroyed with:

    fl = fl_destroy( fl );

`fl_destroy()` destroys both the descriptor and the
storage. `fl_destroy_storage()` is used to destroy the storage, only.

The most ergonomic way of creating a Flexer, is to just start adding
items to it. However the Flexer handle must be initialized to indicate
that the Flexer does not exist yet.

    fs_s fs;
    fl_new_descriptor( &fs );
    fl_push( &fs, data1 );
    fl_push( &fs, data2 );
    ...

Opposite operation for `fl_push()` is `fl_pop()`. `fl_pop()` removes
items from Flexer.

    data = fl_pop( &fs );


Flexer supports a number of queries. User can query container usage,
size, empty, and full status information. 

    fl_used()
    fl_size()
    fl_data()
    fl_end()
    fl_last()
    ...


Flexer can also be used within stack allocated memory. First you have
to have some stack storage available. This can be done with a
convenience macro.

    fl_t fl;
    fl_local_use( fs, buf, 16 );
    fl = &fs;

This will initialize `fs` to stack allocated storage where Flexer has
size of 16. `fl_local_use` uses `fl_use` function for assigning `fs`
to the storage.

When Flexer is taken into use through `fl_use` it is marked as
"local". This means that it will not be freed with `fl_destroy`. Stack
allocated Flexer is automatically changed to a heap allocated Flexer
if Flexer requires resizing. This is quite powerful optimization,
since often stack allocated memory is enough and heap reservation
(which is slowish) is not needed. `fl_destroy` can be called for
Flexer whether its "local" or not. If Flexer is "local", no memory is
released, but Flexer storage reference is set to NULL.


By default Flexer library uses malloc and friends to do heap
allocations. If you define FLEXER_MEM_API, you can use your own memory
allocation functions.

Custom memory function prototypes:
    void* fl_malloc ( size_t size );
    void  fl_free   ( void*  ptr  );
    void* fl_realloc( void*  ptr, size_t size );


See Doxygen docs and `flexer.h` for details about Flexer API. Also
consult the test directory for usage examples.


## Flexer API documentation

See Doxygen documentation. Documentation can be created with:

    shell> doxygen .doxygen


## Examples

All functions and their use is visible in tests. Please refer `test`
directory for testcases.


## Building

Ceedling based flow is in use:

    shell> ceedling

Testing:

    shell> ceedling test:all

User defines can be placed into `project.yml`. Please refer to
Ceedling documentation for details.


## Ceedling

Flexer uses Ceedling for building and testing. Standard Ceedling files
are not in GIT. These can be added by executing:

    shell> ceedling new flexer

in the directory above Flexer. Ceedling prompts for file
overwrites. You should answer NO in order to use the customized files.
