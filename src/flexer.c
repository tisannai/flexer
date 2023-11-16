/**
 * @file   flexer.c
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sat Mar  3 19:07:07 2018
 *
 * @brief  Flexer - Growing container for data.
 *
 */

#define _POSIX_C_SOURCE 200112L

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "flexer.h"


/* clang-format off */

/** @cond flexer_none */
#define fl_true  1
#define fl_false 0

#define fl_smsk            0xFFFFFFFFFFFFFFFEL
#define fl_snor(size)      (((size) & 0x1L) ? (size) + 1 : (size))
#define fl_local( fl )     ( (fl)->size & 0x1L )
#define fm_size( fl )      ( (fl)->size & fl_smsk )
/** @endcond flexer_none */

/* clang-format on */

static fl_t      fl_allocate_descriptor_if( fl_t fl );
static void      fl_set_size( fl_t fl, fl_size_t size );
static void      fl_set_size_and_local( fl_t fl, fl_size_t size, int local );
static void      fl_init( fl_t fl, fl_size_t size, fl_d data, int local );
static fl_size_t fl_align_size( fl_size_t new_size );
static fl_size_t fl_legal_size( fl_size_t size );
static void      fl_resize_to( fl_t fl, fl_size_t new_size );
void             fl_void_assert( void );



/* ------------------------------------------------------------
 * Create and destroy:
 */


fl_t fl_new( fl_t fl )
{
    return fl_new_sized( fl, FL_DEFAULT_SIZE );
}


fl_t fl_new_sized( fl_t fl, fl_size_t size )
{
    fl = fl_allocate_descriptor_if( fl );
    if ( fl == NULL ) {
        /* Memory allocation failure. */
        return fl;
    }

    size = fl_legal_size( size );
    fl_init( fl, size, fl_malloc( size ), 0 );

    return fl;
}


fl_t fl_new_descriptor( fl_t fl )
{
    fl = fl_allocate_descriptor_if( fl );
    if ( fl == NULL ) {
        return fl;
    }
    fl->size = 0;
    fl->used = 0;
    fl->data = NULL;

    return fl;
}


fl_t fl_use( fl_t fl, fl_d mem, fl_size_t size )
{
    fl_assert( size >= FL_MIN_SIZE );
    fl_init( fl, size, mem, 1 );
    memset( fl->data, 0, size );

    return fl;
}


fl_t fl_destroy( fl_t fl )
{
    if ( fl ) {
        fl_destroy_storage( fl );
        fl_free( fl );
    }

    return NULL;
}


void fl_destroy_storage( fl_t fl )
{
    if ( fl == NULL ) {
        return;
    }

    if ( fl->data && !fl_local( fl ) ) {
        fl_free( fl->data );
    }

    fl->data = NULL;
    fl_set_size( fl, 0 );
}


void fl_resize( fl_t fl, fl_size_t new_size )
{
    new_size = fl_legal_size( new_size );
    if ( new_size >= fl->used )
        fl_resize_to( fl, new_size );
}


void fl_add( fl_t fl, const fl_d item, fl_size_t size )
{
    if ( fl->used + size > fm_size( fl ) ) {
        fl_resize( fl, fm_size( fl ) * 2 );
    }
    memcpy( (char*)( fl->data ) + fl->used, item, size );
    fl->used += size;
}


void fl_push( fl_t fl, const fl_d item, fl_size_t size )
{
    if ( fl->data == NULL ) {
        fl_new( fl );
    }

    if ( fl->used + size > fm_size( fl ) ) {
        fl_resize( fl, fm_size( fl ) * 2 );
    }
    memcpy( (char*)( fl->data ) + fl->used, item, size );
    fl->used += size;
}


fl_d fl_pop( fl_t fl, fl_size_t size )
{
    if ( size <= fl->used ) {
        fl->used -= size;
        return (fl_d)( (char*)( fl->data ) + fl->used );
    } else {
        return NULL;
    }
}


fl_d fl_alloc( fl_t fl, fl_size_t size )
{
    fl_d ret;

//     if ( fl->data == NULL ) {
//         fl_new( fl );
//     }

    if ( fl->used + size > fm_size( fl ) ) {
        fl_resize( fl, fm_size( fl ) * 2 );
    }
//     memcpy( (char*)( fl->data ) + fl->used, item, size );
    ret = (char*)( fl->data ) + fl->used;
    fl->used += size;

    return ret;
}


void fl_reset( fl_t fl )
{
    fl->used = 0;
}


void fl_clear( fl_t fl )
{
    fl->used = 0;
    memset( fl->data, 0, fm_size( fl ) );
}


fl_s fl_duplicate( fl_t fl )
{
    fl_s dup;

    fl_new_sized( &dup, fm_size( fl ) );
    dup.used = fl->used;
    memcpy( dup.data, fl->data, fl->used );

    return dup;
}


/* ------------------------------------------------------------
 * Queries:
 */

fl_size_t fl_used( fl_t fl )
{
    return fl->used;
}


fl_size_t fl_size( fl_t fl )
{
    return fm_size( fl );
}


fl_d fl_data( fl_t fl )
{
    return fl->data;
}


fl_d fl_end( fl_t fl )
{
    if ( fl->used > 0 ) {
        return (char*)( fl->data ) + fl->used;
    } else {
        return NULL;
    }
}


fl_d fl_last( fl_t fl, size_t size )
{
    if ( fl->used > 0 ) {
        return ( (char*)( fl->data ) + fl->used - size );
    } else {
        return NULL;
    }
}


int fl_is_empty( fl_t fl )
{
    if ( fl->data == NULL ) {
        return fl_false;
    }

    if ( fl->used == 0 ) {
        return fl_true;
    } else {
        return fl_false;
    }
}


int fl_is_full( fl_t fl )
{
    if ( fl->data == NULL ) {
        return fl_false;
    }

    if ( fl->used >= fm_size( fl ) ) {
        return fl_true;
    } else {
        return fl_false;
    }
}


void fl_set_local( fl_t fl, int val )
{
    if ( val != 0 ) {
        fl->size = fl->size | 0x1L;
    } else {
        fl->size = fl->size & fl_smsk;
    }
}


int fl_get_local( fl_t fl )
{
    return fl_local( fl );
}



/* ------------------------------------------------------------
 * Internal support:
 */


/**
 * Allocate Flexer descriptor if fl is non-NULL.
 *
 * Return NULL on allocation failure.
 *
 * @param fl Flexer or NULL.
 *
 * @return Flexer or NULL.
 */
static fl_t fl_allocate_descriptor_if( fl_t fl )
{
    if ( fl == NULL ) {
        fl = fl_malloc( sizeof( fl_s ) );
    }
    return fl;
}


/**
 * Set size for Flexer, without touching the "local" info.
 *
 * @param fl    Flexer.
 * @param size  Size.
 */
static void fl_set_size( fl_t fl, fl_size_t size )
{
    int local;

    local = fl_get_local( fl );
    fl->size = size;
    fl_set_local( fl, local );
}


/**
 * Set size and local for Flexer.
 *
 * @param fl     Flexer.
 * @param size   Size.
 * @param local  Local info.
 */
static void fl_set_size_and_local( fl_t fl, fl_size_t size, int local )
{
    fl->size = size;
    fl_set_local( fl, local );
}


/**
 * Initialize Flexer struct to given size and local mode.
 *
 * @param fl    Flexer.
 * @param size  Size.
 * @param local Local mode.
 */
static void fl_init( fl_t fl, fl_size_t size, fl_d data, int local )
{
    fl_set_size_and_local( fl, size, local );
    fl->used = 0;
    fl->data = data;
}


/**
 * Align reservation size for 4k and bigger.
 *
 * Small reservations are not effected.
 *
 * @param new_size Size to align, if needed.
 *
 * @return Valid size.
 */
static fl_size_t fl_align_size( fl_size_t new_size )
{
    if ( new_size >= 4096 ) {
        if ( new_size == 4096 ) {
            new_size = 4096 - sizeof( fl_s );
        } else {
            new_size = ( ( ( new_size >> 12 ) + 1 ) << 12 ) - sizeof( fl_s );
        }
    }

    return new_size;
}


/**
 * Convert size to a legal Flexer size.
 *
 * @param size Requested size.
 *
 * @return Legal size.
 */
static fl_size_t fl_legal_size( fl_size_t size )
{
    size = fl_snor( size );

    if ( size < FL_MIN_SIZE ) {
        size = FL_MIN_SIZE;
    }

    return fl_align_size( size );
}


/**
 * Resize Flexer to requested size.
 *
 * @param fp       Flexer reference.
 * @param new_size Requested size.
 */
static void fl_resize_to( fl_t fl, fl_size_t new_size )
{
    if ( fl_get_local( fl ) ) {

        fl->data = (fl_t)fl_malloc( new_size );

    } else {

        fl_size_t old_size = fm_size( fl );
        fl->data = (fl_t)fl_realloc( fl->data, new_size );

        if ( new_size > old_size ) {
            /* Clear newly allocated memory. */
            memset( (char*)( fl->data ) + old_size, 0, ( new_size - old_size ) );
        }
    }

    fl->size = new_size;

    /* NOTE: Setting to non-local is not needed, since size is already
     * an even value. It is here only for clarity. */
    fl_set_local( fl, 0 );
}


/**
 * Disabled (void) assertion.
 */
void fl_void_assert( void ) // GCOV_EXCL_LINE
{
}
