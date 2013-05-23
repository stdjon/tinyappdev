#ifndef TFR_HEADER_H
#define TFR_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

/** \file This file defines a lightweight unit-testing framework.
 */


/*----------------------------------------------------------------------------*/
/* Macros/Public API */

/** Declares a test suite.
 *  name_ - the name of the suite - should be a valid C/C++ string literal.
 *  open_ - function (returning TFR_Bool) to call before each unit test (can be 0).
 *  close_ - function (returning void) to call after each unit test (can be 0).
 */
#define TFR_SUITE_DECLARE_M(name_, open_, close_) \
    do { \
        static TFR_Suite suite = { name_, }; \
        suite.open = open_; \
        suite.close = close_; \
        TFR_add(&suite); \
    } while(0)


/** Adds a unit test to the most recently declared suite.
 *  name_ - function (returning TFR_Bool) to call to carry out a test.
 */
#define TFR_SUITE_ADD_M(name_) \
    do { \
        TFR_Unit *unit = TFR_next_unit(); \
        if(unit) { \
            unit->name = #name_; \
            unit->func = name_; \
        } \
    } while(0)


/*----------------------------------------------------------------------------*/
/* Types */

enum
{
    /** The maximum number of unit tests per test suite */
    TFR_MAX_UNITS_PER_SUITE = 32, 
};


#ifdef __cplusplus
typedef bool TFR_Bool;
const TFR_Bool TFR_false = false;
const TFR_Bool TFR_true = true;
#else/*__cplusplus*/
typedef enum TFR_Bool
{
    TFR_false,
    TFR_true,

} TFR_Bool;
#endif/*__cplusplus*/


typedef const char *TFR_String;


/** Unit test
 */
typedef struct TFR_Unit
{
    TFR_String name; /**< test name */
    TFR_Bool (*func)(); /**< test function */

} TFR_Unit;


/** Test Suite.
 *  A test suite represents a set of unit tests.
 *  This structure has no methods, as it is intended to be implemented as a literal.
 */
typedef struct TFR_Suite
{
    TFR_String name; /**< name of the test */
    TFR_Bool (*open)(); /**< test constructor */
    void (*close)(); /**< test destructor */
    TFR_Unit unit[TFR_MAX_UNITS_PER_SUITE];

    const struct TFR_Suite *next; /**< \internal the next test in the suite */

} TFR_Suite;


/** Test .
 *  A list of TFR_Suites.
 */
typedef struct TFR_Test
{
    TFR_String title; /**< \internal Title */
    TFR_String focus; /**< \internal Focus (only the named suite will execute) */
    const TFR_Suite *first; /**< \internal The first test in the suite. */
    const TFR_Suite **hook; /**< \internal The address of the next pointer of the last suite. */
    int verbosity; /**< \internal The suite verbosity. (The higher, the more verbose.) */
    TFR_Bool help; /**< \internal Show help and exit */

} TFR_Test;


/*----------------------------------------------------------------------------*/
/* Functions */

/** Run all test suites.
 *  \return \c true if all tests pass.
 *  Called by the main() if driver.c is compiled in.
 */
TFR_Bool TFR_run();


/** User init for driver.
 *  This is not defined in the library, it is called by main() if driver.c is
 *  compiled in. Create a function with this name and use it to declare test
 *  suites and add unit tests to them with the TFR_SUITE_*() macros.
 */
void TFR_main(int argc, char **argv);


typedef void (*TFR_ExitHandler)(void*);

/** Register exit handler
 */
void TFR_on_exit(TFR_ExitHandler exit, void *param);


/** Unit test output.
 */
void TFR_trace(int verbosity, TFR_String format, ...);


/** Unit test check boolean condition and output result.
 */
#define TFR_check(V_, EXPR_) TFR_do_check(V_, #EXPR_, EXPR_)



/*----------------------------------------------------------------------------*/
/* Internal Functions */
/* You generally don't need to call any of these from user code!
 */

/** Set verbosity level for the test run. Higher value -> more debug prints.
 */
void TFR_set_verbosity(int verbosity);


/** Get current verbosity level.
 */
int TFR_get_verbosity();


/** Set a title for the test run. (Printed at the top of the test output report.)
 */
void TFR_set_title(TFR_String title);


/** Set focus on a specific suite (specified by name) - only the suite matching
 *  the name will be executed. (This is primarily for faster iteration on
 *  specific tests while developing, if running all the tests is taking too
 *  long.) The string comparison is case insensitive, for convenince.
 */
void TFR_set_focus(TFR_String name);


/** Commandline handling for test suites.
 */
TFR_Bool TFR_args(int *argc, char ***argv);


/*----------------------------------------------------------------------------*/

/** Add a test suite. (Called internally by TFR_SUITE_DECLARE_M(), so user
 *  generally won't need to call this.
 */
TFR_Bool TFR_add(TFR_Suite *suite);

/** Advance to next unit in a suite, called internally by TFR_SUITE_ADD_M()
 */
TFR_Unit *TFR_next_unit();

/** Back end to TFR_check()
 */
TFR_Bool TFR_do_check(int verbosity, const char *expr, TFR_Bool result);

#ifdef __cplusplus
}
#endif/*__cplusplus*/
#endif/*TFR_HEADER_H*/
