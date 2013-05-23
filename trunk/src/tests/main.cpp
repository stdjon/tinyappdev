#include "testframe/test.h"

// Add tests/test sets (in subdirectories); include headers here...
#include "tests/pml.h"


//------------------------------------------------------------------------------
// This is a stub test suite to demonstrate that the unit test framework is
// working properly, and to serve as a reference for new suites.

TFR_Bool open() {
    TFR_trace(2, "open()\n");
    return true;
}


void close() {
    TFR_trace(2, "close()\n");
}


TFR_Bool test1() {
    TFR_trace(2, "test1()\n");
    return true;
}


TFR_Bool test2() {
    TFR_trace(2, "test2()\n");
    return true;
}


//------------------------------------------------------------------------------
/** Test Framework entry point.
 */
void TFR_main(int argc, char **argv) {

    using namespace tests;

    TFR_set_title("tinyappdev");

    // TODO: more tests here!

    c_declare_pml_tests();
    pml::declare_pml_tests();

}


