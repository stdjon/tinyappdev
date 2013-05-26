#include "testframe/test.h"

#ifdef TFR_USE_MINICONF_S
#include "miniconf/config.h"
#include "miniconf/util.h"
#endif//TFR_USE_MINICONF_S

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// state variables
static int TFR_counter_S;
static TFR_Suite *TFR_suite_S;
static TFR_Test TFR_test_S;
static TFR_ExitHandler TFR_exit_handler_S;
static void *TFR_exit_handler_param_S;


static int TFR_stricmp(TFR_String a, TFR_String b)
{
    /* TODO: this is not a particularly great implementation, however this
     * function is only used to compare suite names if TFR_set_focus() was used,
     * so the limit on string length shouldn't be an issue. (Suite names being
     * generally 5-10 characters in length.)
     */

    enum { ARBITRARY_BUFFER_SIZE = 100, };
    char a_[ARBITRARY_BUFFER_SIZE] = {0};
    char b_[ARBITRARY_BUFFER_SIZE] = {0};

    assert(
        (strlen(a) + 1) < ARBITRARY_BUFFER_SIZE &&
        (strlen(b) + 1) < ARBITRARY_BUFFER_SIZE);

    for(int i = 0; a[i]; i++) { a_[i] = tolower(a[i]); }
    for(int i = 0; b[i]; i++) { b_[i] = tolower(b[i]); }

    return strcmp(a_, b_);
}



static TFR_Bool TFR_open(const TFR_Suite *test)
{
    return test->open ?
        test->open() :
        TFR_true;
}


static void TFR_close(const TFR_Suite *test)
{
    if(test->close)
    {
        test->close();
    }
}


// Handles the display of help text/usage.
#ifdef TFR_USE_MINICONF_S
TFR_Bool TFR_usage()
{
    if(TFR_test_S.help)
    {
        TFR_String title = TFR_test_S.title;

        printf("A unit test suite");
        printf(title ? " for %s\n\n" : "\n\n", title);

        cfgDefaultReportOptions opt = {
            /*.indent =*/ 18
        };
        cfg_report(cfg_default_report, &opt);
        printf("\n\n");
    }

    cfg_exit();

    return TFR_test_S.help;
}
#endif//TFR_USE_MINICONF_S

//------------------------------------------------------------------------------
// Public API

void TFR_set_verbosity(int verbosity)
{
    TFR_test_S.verbosity = verbosity;
}


int TFR_get_verbosity()
{
    return TFR_test_S.verbosity;
}


void TFR_set_title(TFR_String title)
{
    TFR_test_S.title = title;
}


void TFR_set_focus(TFR_String focus)
{
    TFR_test_S.focus = focus;
}


TFR_Bool TFR_add(TFR_Suite *suite)
{
    assert(suite);

    if( TFR_test_S.focus &&
        TFR_stricmp(TFR_test_S.focus, suite->name))
    {
        TFR_suite_S = suite;
        TFR_counter_S = 0;
        return TFR_false;
    }

    if(0 == TFR_test_S.hook)
    {
        TFR_test_S.hook = &TFR_test_S.first;
    }

    *(TFR_test_S.hook) = suite;
    TFR_test_S.hook = &suite->next;
    suite->next = 0;

    TFR_suite_S = suite;
    TFR_counter_S = 0;

    return TFR_true;
}


TFR_Bool TFR_run()
{
    const TFR_Suite *test = TFR_test_S.first;
    int suite_tests = 0;
    int suite_success = 0;

#ifdef TFR_USE_MINICONF_S
    if(TFR_usage())
    {
        return TFR_true;
    }
#endif//TFR_USE_MINICONF_S

    printf(
        "================================================================================\n"
        " %s Test Suite\n", TFR_test_S.title ? TFR_test_S.title : "Anonymous");

    while(test)
    {
        TFR_Bool result = TFR_false;
        suite_tests++;

        if(0 == TFR_test_S.verbosity)
        {
            if(1 == suite_tests)
            {
                printf(
                    "================================================================================\n");
            }

            int len = strlen(test->name);
            for(int i = 0; i < 12; i++)
            {
                putchar( (i < len) ? test->name[i] : ' ');
            }
        }
        else
        {
            TFR_trace(0,
                "================================================================================\n"
                "  %s\n"
                "--------------------------------------------------------------------------------\n",
                test->name);
        }

        int unit_tests = 0;
        int unit_success = 0;

        for(int i = 0; test->unit[i].func && i < TFR_MAX_UNITS_PER_SUITE; i++)
        {
            unit_tests++;
            TFR_trace(1, "* %s:", test->unit[i].name);
            TFR_trace(2, "\n");

            if(TFR_open(test))
            {
                if(test->unit[i].func())
                {
                    TFR_trace(1, "  Success\n");

                    unit_success++;
                }
                else
                {
                    TFR_trace(1, "  Failed\n");
                }

                TFR_close(test);

                TFR_trace(2,
                    "--------------------------------------------------------------------------------\n");
            }
            else
            {
                TFR_trace(1, "  Failed to open\n");
            }

        }

        result = (unit_success == unit_tests);

        if(0 == TFR_test_S.verbosity)
        {
            TFR_trace(0, " (%2u/%2u) ", unit_success, unit_tests);

            int width = 57;
            int succ = ((float)unit_success / unit_tests) * width;
            int i;

            putchar('[');
            for(i = 0; i < succ; i++) { putchar('#'); }
            for(i = 0; i < (width - succ); i++) { putchar(' '); }
            putchar(']');
            putchar('\n');
        }
        else
        {
            TFR_trace(0, "  %s (%u of %u)\n",
                result ? "Success" : "Failure", unit_success, unit_tests);
        }

        if(result) { suite_success++; }

        test = test->next;
    }

    TFR_trace(0,
        "================================================================================\n"
        "  %u of %u tests succeeded\n"
        "================================================================================\n",
        suite_success, suite_tests);


    // this was potentially strdup()ed in TFR_args
    if(TFR_test_S.focus)
    {
        free((void*)TFR_test_S.focus);
    }

    if(TFR_exit_handler_S)
    {
        TFR_exit_handler_S(TFR_exit_handler_param_S);
    }

    return (suite_success == suite_tests);
}


void TFR_on_exit(TFR_ExitHandler exit, void *param) {
    TFR_exit_handler_S = exit;
    TFR_exit_handler_param_S = param;
}

void TFR_trace(int verbosity, TFR_String format, ...)
{
    assert(format);

    if(verbosity <= TFR_test_S.verbosity)
    {
        va_list list;

        va_start(list, format);

        vprintf(format, list);

        va_end(list);
    }
}


TFR_Bool TFR_do_check(int verbosity, const char *expr, TFR_Bool result)
{
    TFR_trace(verbosity, "%s: %s\n", expr, result ? "ok" : "fail");
    return result;
}


/*----------------------------------------------------------------------------*/
/* Command line argument handling
 * NB: this has no effect unless TFR_USE_MINICONF_S is #defined.
 */

TFR_Bool TFR_args(int *argc, char ***argv)
{
#ifdef TFR_USE_MINICONF_S
    assert(argc);
    assert(argv);

    cfgOption options[] =
    {
        cfgSWITCH_M("help", "h", "Show this help."),
        cfgSWITCH_M("?", 0, "Synonym for -help"),
        cfgSTRING_M("argfile", "A", 0, "Read additional arguments from <filename>"),
        cfgSTRING_M("focus", "f", 0, "Run tests on one suite only."),
        cfgINT_M("verbosity", "v", 0, "Verbosity (output trace) level. Higher for more info."),
        cfgEND_M
    };

    TFR_Bool result = TFR_false;

    if(cfg_init())
    {
        result = TFR_true;

        if(cfg_add_options(options) && 
            cfg_read_args(argc, argv))
        {
            TFR_String argfile = cfg_string("argfile");
            if(argfile) { cfg_read_file(argfile); }

            TFR_test_S.verbosity = cfg_int("verbosity");
            TFR_test_S.focus = cfg_string("focus");
            TFR_test_S.help = cfg_switch("help") || cfg_switch("?");
        }

        if(TFR_test_S.focus)
        {
            /* Copy the string - it will be freed at end of TFR_run() */
            TFR_test_S.focus = strdup(TFR_test_S.focus);
        }

        /* cfg_exit() is called from TFR_usage */
    }

    return result;

#else/*TFR_USE_MINICONF_S*/
    return TFR_true;
#endif/*TFR_USE_MINICONF_S*/
}


TFR_Unit *TFR_next_unit()
{
    TFR_Unit *u = 0;
    if(TFR_suite_S)
    {
        u = &TFR_suite_S->unit[TFR_counter_S];
        TFR_counter_S++;
    }
    return u;
}


