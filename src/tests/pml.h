#ifndef TESTS_PML_H
#define TESTS_PML_H

#include "testframe/test.h"

//------------------------------------------------------------------------------
// Declarations of PML unit tests go here.

#ifdef __cplusplus
namespace tests {
namespace pml {


// pml/malloc.cpp - test malloc

void declare_pml_tests();


} // namespace pml
} // namespace tests

extern "C" {
#endif//__cplusplus

// pml/malloc.c

void c_declare_pml_tests();

#ifdef __cplusplus
} // extern "C"
#endif//__cplusplus


#endif//TESTS_PML_H
