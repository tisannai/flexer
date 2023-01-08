#include "unity.h"
#include "flexer.h"
#include <string.h>
#include <unistd.h>


void test_basics( void )
{
    fl_s  fs;
    fl_t  fl;
    fl_s  dup;
    char* text = "text";
    fl_d  ret;
    int text_len;

    text_len = strlen( text ) + 1;

    fl = fl_new( &fs );
    TEST_ASSERT_EQUAL( FL_DEFAULT_SIZE, fl_size( fl ) );
    TEST_ASSERT_EQUAL( 1, fl_is_empty( fl ) );
    TEST_ASSERT_EQUAL( 0, fl_is_full( fl ) );

    fl_destroy_storage( fl );
    TEST_ASSERT_EQUAL( NULL, fl->data );
    TEST_ASSERT_EQUAL( 0, fl_size( fl ) );

    fl = fl_new_sized( NULL, 12 );
    TEST_ASSERT_EQUAL( 12, fl_size( fl ) );
    TEST_ASSERT_EQUAL( 0, fl_used( fl ) );

    fl_push( fl, text, text_len );
    TEST_ASSERT_EQUAL( 12, fl_size( fl ) );
    TEST_ASSERT_EQUAL( text_len, fl_used( fl ) );

    fl_reset( fl );
    TEST_ASSERT_EQUAL( 0, fl_used( fl ) );
    fl_resize( fl, FL_DEFAULT_SIZE );
    TEST_ASSERT_EQUAL( FL_DEFAULT_SIZE, fl_size( fl ) );

    fl_push( fl, text, text_len );
    TEST_ASSERT_EQUAL( FL_DEFAULT_SIZE, fl_size( fl ) );
    TEST_ASSERT_EQUAL( text_len, fl_used( fl ) );

    ret = fl_pop( fl, text_len );
    TEST_ASSERT_EQUAL_STRING( (char*)text, (char*)ret );


    fl_push( fl, text, text_len );
    fl_push( fl, text, text_len );
    dup = fl_duplicate( fl );
    TEST_ASSERT( fl_size( &dup ) == fl_size( fl ) );
    TEST_ASSERT( fl_used( &dup ) == fl_used( fl ) );
    TEST_ASSERT( memcmp( dup.data, fl->data, 2*text_len ) == 0 );

    fl = fl_destroy( fl );
    fl_destroy_storage( &dup );

    /* Destroy an empty Flexer. */
    fl_destroy( fl );
    fl_destroy_storage( fl );
}
