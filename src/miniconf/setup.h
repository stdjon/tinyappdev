#ifndef cfgSETUP_H
#define cfgSETUP_H

/** \file
 *  \ingroup miniconf
 *
 *  This header attempts to contain (and describe) all miniconf's dependencies
 *  on external libraries.
 *  If you want to port miniconf to another codebase, you can edit this header
 *  to include any headers needed and adjust the typedefs and macros below to
 *  tune the library's behaviour to your needs.
 *
 *  If you don't want to (or can't) modify the miniconf source code, you can
 *  copy this file and edit the copy instead. The macro MINICONF_SETUP_H can be
 *  defined as the path to the user copy which will be included in config.h by
 *  an #include directive. For example:
 *
 *    #define MINICONF_SETUP_H "user_miniconf_setup.h"
 *    #include "miniconf/config.h"
 */


/*----------------------------------------------------------------------------*/
/* Language configuration */

/* C++ compatibility (specify C linkage) */
#ifdef __cplusplus
    #define cfgCPP_S
#endif/*__cplusplus*/


/* Detect C99 compatibility (for use of C99-specific syntax). */
#if (defined(__STDC__) && __STDC_VERSION__ >= 199901L)
    #define cfgC99_S
#endif/*...*/


/*----------------------------------------------------------------------------*/
/*  Headers */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef cfgC99_S
#include <stdint.h> /* for intptr_t */
#endif/*cfgC99_S*/

/*----------------------------------------------------------------------------*/
/* Macros */

/** Allocate SIZE_ bytes of memory (with optional hint provided for debugging
 *  purposes)
 */
#define cfgALLOC_M(SIZE_, HINT_) malloc(SIZE_)

/** Free memory previously allocated (by cfgALLOC_M()).
 */
#define cfgFREE_M(PTR_) free(PTR_)

/** Assertion macro. (Halt execution if EXPR_ is false.)
 */
#define cfgASSERT_M(EXPR_) assert(EXPR_)


/** Built time assertion.
 */
#define cfgBTASSERT_M(EXPR_, REASON_) \
    typedef int REASON_[EXPR_ ? 1 : -1];


/* This allocation hint macro can be used to modify the hint string - for
 * example: to expand __FILE__ and __LINE__ and concantenate them into the hint
 * string.
 */
#define ocnHINT_M(HINT_) ocnHINT1_M(HINT_, __FILE__, __LINE__)

/* implementation details... */
#define ocnHINT1_M(HINT_, FILE_, LINE_) ocnHINT2_M(HINT_, FILE_, LINE_)
#define ocnHINT2_M(HINT_, FILE_, LINE_) (FILE_ "(" #LINE_ "): " HINT_)


/*----------------------------------------------------------------------------*/
/* Types */

/* Boolean */
typedef enum cfgBool
{
    cfg_false,
    cfg_true,

} cfgBool;


/* cfgInt needs to be >= pointer size. */
#ifdef cfgC99_S
typedef intptr_t cfgInt;
#else/*cfgC99_S*/
typedef long cfgInt;
#endif/*cfgC99_S*/

typedef float cfgFloat;

typedef char cfgChar; /* Character type */

/* String types */
typedef cfgChar const *cfgString;
typedef cfgChar *cfgMutableString;

/* Magnitude type (unsigned) */
typedef size_t cfgSize;

typedef FILE cfgFILE; /* file type (see also cfg_fopen et al below...) */


/*----------------------------------------------------------------------------*/
/* Limits */

/* 100 options ought to be enough for anybody!
 * Nevertheless, limits can be adjusted below...
 */

enum
{
    /** Maximum number of options which can be simultaneously supported. */
    cfgMAX_OPTION_COUNT = 100,

    /** Maximum number of argv entries which can be processed by
     *  cfg_read_args() */
    cfgMAX_ARG_COUNT = 100,

    /** Maximum number of argv arrays we can maintain concurrently.
     *  (ie maximum number of times we can call cfg_read_args() */
    cfgMAX_ARG_BLOCK_COUNT = 50,
};


/*----------------------------------------------------------------------------*/
/* Standard C functions */

/** By editing the macros below, the standard functions used by miniconf can be
 *  replaced with equivalent functions, if so desired. (A custom file system
 *  could be used, for example).
 *
 *  It is preferable to define these as parameterless macros, as they can be
 *  used more flexibly than function-style macros (the address of the replaced
 *  functions can be taken, etc...)
 */

#define cfg_atof atof
#define cfg_atoi atoi
#define cfg_fclose fclose
#define cfg_fopen fopen
#define cfg_fgets fgets
#define cfg_printf printf
#define cfg_strcmp strcmp
#define cfg_strlen strlen
#define cfg_strncpy strncpy


#endif/*cfgSETUP_H*/
