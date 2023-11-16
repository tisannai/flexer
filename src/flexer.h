#ifndef FLEXER_H
#define FLEXER_H

/**
 * @file   flexer.h
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sun Jan  1 23:01:09 2023
 *
 * @brief  Flexer - Growing container for data.
 *
 */

#ifndef SIXTEN_STD_INCLUDE
#include <stdlib.h>
#include <stdint.h>
#endif


#ifndef FLEXER_NO_ASSERT
#include <assert.h>
/** Default assert. */
#define fl_assert assert
#else
/** Disabled assertion. */
#define fl_assert( cond ) (void)( ( cond ) || fl_void_assert() )
#endif


#ifndef FL_DEFAULT_SIZE
/** Default size for Flexer. */
#define FL_DEFAULT_SIZE 16
#endif

/** Minimum size for pointer array. */
#define FL_MIN_SIZE 2

/** Outsize Flexer index. */
#define FL_NOT_INDEX -1


/** Size type. */
typedef uint64_t fl_size_t;

/** Position type. */
typedef int64_t fl_pos_t;

/** Data pointer type. */
typedef void* fl_d;


/**
 * Flexer struct.
 *
 * NOTE: "size" is strictly forbidden to be used directly, because it
 * includes the "local/non-local" information in LSB. Use po_size()
 * instead.
 */
struct fl_struct_s
{
    fl_size_t size; /**< Reservation size for data (N mod 2==0). */
    fl_size_t used; /**< Used count for data. */
    fl_d      data; /**< Pointer to data. */
};
typedef struct fl_struct_s fl_s; /**< Flexer struct. */
typedef fl_s*              fl_t; /**< Flexer. */
typedef fl_t*              fl_p; /**< Flexer reference. */


/**
 * Make allocation for local (stack) Flexer. NOTE: User must make sure
 * the allocation size is valid.
 */
#define fl_local_use( fs, buf, size )                 \
    fl_s fs;                                          \
    /* Legalize the size, i.e. make it even. */       \
    fl_d buf[ ( ( ( ( size - 1 ) / 2 ) + 1 ) * 2 ) ]; \
    fl_use( &fs, buf, ( ( ( ( size - 1 ) / 2 ) + 1 ) * 2 ) )


/* clang-format off */

#ifdef FLEXER_USE_MEM_API

/*
 * FLEXER_USE_MEM_API allows to use custom memory allocation functions,
 * instead of the default: fl_malloc, fl_free, fl_realloc.
 *
 * If FLEXER_USE_MEM_API is used, the user must provide implementation for
 * the above functions and they must be compatible with malloc
 * etc. Also Flexer assumes that fl_malloc sets all new memory to
 * zero.
 *
 * Additionally user should compile the library by own means.
 */

extern void* fl_malloc( size_t size );
extern void  fl_free( void* ptr );
extern void* fl_realloc( void* ptr, size_t size );

#else /* FLEXER_USE_MEM_API */


#    if SIXTEN_USE_MEM_API == 1

#        define fl_malloc  st_alloc
#        define fl_free    st_del
#        define fl_realloc st_realloc


#    else /* SIXTEN_USE_MEM_API == 1 */

/* Default to common memory management functions. */

/** Reserve memory. */
#        define fl_malloc( size ) calloc( size, 1 )

/** Release memory. */
#        define fl_free free

/** Re-reserve memory. */
#        define fl_realloc realloc

#    endif /* SIXTEN_USE_MEM_API == 1 */

#endif /* FLEXER_USE_MEM_API */

/* clang-format on */


/* ------------------------------------------------------------
 * Create and destroy:
 */


/**
 * Create Flexer container with default size (FL_DEFAULT_SIZE).
 *
 * If fl is NULL, descriptor is allocated from heap. This
 * type of descriptor must be freed by the user after use.
 *
 * @param fl  Flexer or NULL.
 *
 * @return Flexer.
 */
fl_t fl_new( fl_t fl );


/**
 * Create Flexer with given size.
 *
 * If fl is NULL, descriptor is allocated from heap. This
 * type of descriptor must be freed by the user after use.
 *
 * Minimum size is FL_MINSIZE. Size is forced if given size is too
 * small.
 *
 * @param fl   Flexer or NULL.
 * @param size Initial size.
 *
 * @return Flexer.
 */
fl_t fl_new_sized( fl_t fl, fl_size_t size );


/**
 * Initialize the Flexer as empty.
 *
 * If fl is NULL, descriptor is allocated from heap. This
 * type of descriptor must be freed by the user after use.
 *
 * @param fl Flexer descriptor or NULL.
 *
 * @return Initialized Flexer.
 */
fl_t fl_new_descriptor( fl_t fl );


/**
 * Use existing memory allocation for Flexer.
 *
 * @param fl   Flexer.
 * @param mem  Allocation for Flexer data.
 * @param size Allocation size (in bytes).
 *
 * @return Flexer.
 */
fl_t fl_use( fl_t fl, fl_d mem, fl_size_t size );


/**
 * Destroy Flexer.
 *
 * @param fl Flexer.
 *
 * @return NULL;
 */
fl_t fl_destroy( fl_t fl );


/**
 * Destroy Flexer storage (data).
 *
 * @param fl Flexer.
 */
void fl_destroy_storage( fl_t fl );


/**
 * Resize Flexer to new_size.
 *
 * If new_size is smaller than usage, no action is performed.
 *
 * @param fl       Flexer.
 * @param new_size Requested size.
 */
void fl_resize( fl_t fl, fl_size_t new_size );


/**
 * Add item to container end.
 *
 * If Flexer data must be already allocated.
 *
 * @param fl   Flexer.
 * @param item Item to add.
 * @param size Number of bytes to add.
 */
void fl_add( fl_t fl, const fl_d item, fl_size_t size );


/**
 * Push (add) item to container end.
 *
 * If Flexer data is NULL, Flexer is created and item is added. Flexer
 * is resized if these is no space available.
 *
 * @param fl   Flexer.
 * @param item Item to add.
 * @param size Number of bytes to add.
 */
void fl_push( fl_t fl, const fl_d item, fl_size_t size );


/**
 * Pop (remove) item from container end.
 *
 * @param fl   Flexer.
 * @param size Number of bytes to remove.
 *
 * @return Popped (removed) item (or NULL).
 */
fl_d fl_pop( fl_t fl, fl_size_t size );


/**
 * Get allocation from container.
 *
 * @param fl   Flexer.
 * @param size Number of bytes to allocate.
 */
fl_d fl_alloc( fl_t fl, fl_size_t size );


/**
 * Reset Flexer to empty.
 *
 * @param fl Flexer.
 */
void fl_reset( fl_t fl );


/**
 * Clear Flexer, i.e. reset and clear data.
 *
 * @param fl Flexer.
 */
void fl_clear( fl_t fl );


/**
 * Duplicate Flexer.
 *
 * @param fl Flexer to duplicate.
 *
 * @return Duplicated flexer descriptor.
 */
fl_s fl_duplicate( fl_t fl );



/* ------------------------------------------------------------
 * Queries:
 */


/**
 * Return count of container usage.
 *
 * @param fl Flexer.
 *
 * @return Usage count (in bytes).
 */
fl_size_t fl_used( fl_t fl );


/**
 * Return Flexer reservation size.
 *
 * @param fl Flexer.
 *
 * @return Reservation size.
 */
fl_size_t fl_size( fl_t fl );


/**
 * Return Flexer container data.
 *
 * @param fl Flexer.
 *
 * @return Data array.
 */
fl_d fl_data( fl_t fl );


/**
 * Return container endpoint address (pointer).
 *
 * @param fl Flexer.
 *
 * @return Data array end address, or NULL.
 */
fl_d fl_end( fl_t fl );


/**
 * Return last container item.
 *
 * @param fl    Flexer
 * @param size  Item size.
 *
 * @return Item, or NULL.
 */
fl_d fl_last( fl_t fl, size_t size );


/**
 * Return empty status.
 *
 * @param fl Flexer.
 *
 * @return 1 if empty, 0 if not.
 */
int fl_is_empty( fl_t fl );


/**
 * Return full status.
 *
 * @param fl Flexer.
 *
 * @return 1 if full, 0 if not.
 */
int fl_is_full( fl_t fl );


/**
 * Set Flexer as local.
 *
 * @param fl  Flexer.
 * @param val Local, non-zero means local.
 */
void fl_set_local( fl_t gr, int val );


/**
 * Return Flexer local mode.
 *
 * @param fl Flexer.
 *
 * @return 1 if local, else 0.
 */
int fl_get_local( fl_t fl );


void fl_void_assert( void );


#endif
