#ifndef cfgCONFIG_H
#define cfgCONFIG_H

/** \file
 *  \ingroup miniconf
 *
 *  miniconf - a simple portable configuration library.
 *  This library implements configuration option support. Options are strongly
 *  typed, and are specifed with a name and (optional) abbreviation and
 *  description. (Both the abbreviation and description can be specified as 0
 *  or NULL if not required.
 *
 *  Command line options supported:
 *
 *    bool: can take form of -switch or +switch (- is true, + is false)
 *    float: -G 98.1
 *    int: -number_of_examples 3
 *    string: -lawfirm "Atkinson, Bristow & Cholmondeley"
 *
 *  Config file/string options (each option is 1 single line):
 *
 *    comment: #comment
 *    bool: -switch or +switch
 *    float: G=9.81
 *    int: number_of_examples=3
 *    string: lawfirm=Atkinson, Bristow & Cholmondeley
 */

/* Include the setup header.
 *
 * If MINICONF_SETUP_H is #defined (as a string constant), it will be used as the path
 * to the setup header. If not, the default setup.h is used instead.
 *
 * In this way, miniconf can be configured without editing its source code by
 * making a copy of setup.h and editing that instead. MINICONF_SETUP_H can then be
 * defined to indicate the user copy of the setup file instead of the default.
 */
#ifdef MINICONF_SETUP_H
    #include MINICONF_SETUP_H
#else/*MINICONF_SETUP_H*/
    #include "miniconf/setup.h"
#endif/*MINICONF_SETUP_H*/


/* specify C linkage on C++ */
#ifdef cfgCPP_S
    #define cfgEXTERN_C_START_M extern "C" {
    #define cfgEXTERN_C_END_M }
#else/*cfgCPP_S*/
    #define cfgEXTERN_C_START_M
    #define cfgEXTERN_C_END_M
#endif/*cfgCPP_S*/

cfgEXTERN_C_START_M

/*----------------------------------------------------------------------------*/
/* Macros */

#ifdef cfgC99_S

/** A switch (boolean option). It defaults to false, so that -xx on the commandline
 *  makes it true (+xx is essentially a no-op)
 */
#define cfgSWITCH_M(NAME_, ABBREV_, DESC_) \
{ \
    .name = (NAME_), \
    .abbrev = (ABBREV_), \
    .desc = (DESC_), \
    .value = { .type = cfgVT_SWITCH, }, \
}


/** An integer option.
 */
#define cfgINT_M(NAME_, ABBREV_, VALUE_, DESC_) \
{ \
    .name = (NAME_), \
    .abbrev = (ABBREV_), \
    .desc = (DESC_), \
    .value = { .type = cfgVT_INT, .u = { .i = (VALUE_) } }\
}


/** A string option.
 */
#define cfgSTRING_M(NAME_, ABBREV_, VALUE_, DESC_) \
{ \
    .name = (NAME_), \
    .abbrev = (ABBREV_), \
    .desc = (DESC_), \
    .value = { .type = cfgVT_STRING, .u = { .s = (VALUE_) } }\
}


/** A float option.
 *  NB: C99 and C++ will allow float options to be defined at file scope, but on
 *  C89, they can only be defined inside a function. This is due to technical
 *  limitations of that particular standard (C89 can't initialize a specific
 *  member of a union, nor call functions before main() is called.)
 */
#define cfgFLOAT_M(NAME_, ABBREV_, VALUE_, DESC_) \
{ \
    .name = (NAME_), \
    .abbrev = (ABBREV_), \
    .desc = (DESC_), \
    .value = { .type = cfgVT_FLOAT, .u = { .f = (VALUE_) } }\
}


#else/*cfgC99_S*/

/* Non-C99 equivalents, far less safe... */

#define cfgSWITCH_M(NAME_, ABBREV_, DESC_) \
    { NAME_, ABBREV_, DESC_, { cfgVT_SWITCH } }

#define cfgINT_M(NAME_, ABBREV_, VALUE_, DESC_) \
    { NAME_, ABBREV_, DESC_, { cfgVT_INT, { VALUE_ } } }

#define cfgSTRING_M(NAME_, ABBREV_, VALUE_, DESC_) \
    { NAME_, ABBREV_, DESC_, { cfgVT_STRING, { (cfgInt)(VALUE_) } } }

#define cfgFLOAT_M(NAME_, ABBREV_, VALUE_, DESC_) \
    { NAME_, ABBREV_, DESC_, { cfgVT_FLOAT, { cfg_float2bitpattern(VALUE_) } } }

#endif/*cfgC99_S*/


/** Marks the end of the option list.
 */
#define cfgEND_M {0}


/*----------------------------------------------------------------------------*/
/* Types */

/** Key type (pointer to const char).
 */
typedef cfgString cfgKey;


/** Enumerates the available types for values.
 */
typedef enum cfgValueType
{
    /* types */

    cfgVT_SWITCH = 0,
    cfgVT_INT,
    cfgVT_STRING,
    cfgVT_FLOAT,

    cfgVT_COUNT, /* keep this as the last non-enumerated member of this enum */
    cfgVT_NOEXIST = 0x00ff, /* non-existent option (invalid type) */

    /* flags */

    /* flag set if this option is set via cfg_read_args() or cfg_read_file() */
    cfgVT_INPUT = 0x0100,

    cfgVT_TYPE_MASK = 0x00ff, /* mask for types above */
    cfgVT_FLAG_MASK = 0xff00, /* mask for flags above */

} cfgValueType;


/** Value (tagged union)
 *  ** BE VERY CAREFUL MODIFYING THIS - non C99 macros will need to be adjusted **
 */
typedef struct cfgValue
{
    cfgValueType type;

    union
    {
        cfgInt i;
        cfgBool b;
        cfgString s;
        cfgFloat f;
    } u;

} cfgValue;


/** Option entry passed into cfg_add_options()
 *  ** BE VERY CAREFUL MODIFYING THIS - non C99 macros will need to be adjusted **
 */
typedef struct cfgOption
{
    cfgKey name; /* Full name of the option (eg: verbosity). */
    cfgKey abbrev; /* Optional abbreviation (eg: v), can be zero/null if not needed. */
    cfgString desc; /*Optional description of the option */
    cfgValue value; /* The option's current/default value. */

} cfgOption;


/*----------------------------------------------------------------------------*/
/* Functions */

/** Initialize the config system.
 */
cfgBool cfg_init();


/** Shutdown the config system. This will free any memory it allocated.
 *  NB: any pointers returned from cfg_string() will be invalid after cfg_exit()
 *  is called - be sure to make copies of any string config values you wish to
 *  keep before calling cfg_exit().
 */
void cfg_exit();


/** Add option definitions to be tested against input.
 *  As long as only new options are added each time, it is safe to call this
 *  function any number of times. (Attempting to define an option with the same
 *  name as an existing option will cause an assertion.)
 *
 *  For example:
 *
 *    cfgOption options[] =
 *    {
 *        cfgFLOAT_M("float", "f", 12.5f),
 *        cfgINT_M("integer", "i", 1),
 *        cfgSTRING_M("string", "s", "Default"),
 *        cfgSWITCH_M("switch", 0), // passing 0 for no abbreviation
 *        cfgEND_M // This *must* be the last entry
 *    };
 *
 *    cfg_add_options(options);
 */
cfgBool cfg_add_options(cfgOption const *options);


/** Read options from argc/argv style parameters (ie from main()).
 *  When this is called, the arguments argc and argv will be modified to contain
 *  any parts of the commandline which remain after not being processed by miniconf.
 *  Any memory allocated to hold a new argv parameter will be freed by cfg_exit().
 *  (NB: We're assuming that signature of main() is as follows - any other
 *    signature will cause casting or pain... :)
 *
 *      int main(int argc, char **argv);
 */
cfgBool cfg_read_args(int *argc, char ***argv);


/** Read options from the file specified.
 */
cfgBool cfg_read_file(cfgString path);


/** Discover the type of an option from its key. Returns cfgVT_NOEXIST if the
 *  option doesn't exist (so this can also be used to see if an option exists).
 */
cfgValueType cfg_type(cfgKey key);


/** Return true if the option was input by the user (via cfg_read_args() or
 *  cfg_read_file()). If the option doesn't even exist to be set, cfg_type() will
 *  return cfgVT_NOEXIST.
 */
cfgBool cfg_input(cfgKey key);


/** Accessors - get config options at runtime.
 *  NB: trying to read from the wrong type causes an assert.
 */
cfgBool cfg_switch(cfgKey key);
cfgInt cfg_int(cfgKey key);
cfgString cfg_string(cfgKey key);
cfgFloat cfg_float(cfgKey key);


/** Accessors - set config options at runtime.
 *  NB: trying to set to the wrong type causes an assert.
 */
void cfg_set_switch(cfgKey key, cfgBool value);
void cfg_set_int(cfgKey key, cfgInt value);
void cfg_set_string(cfgKey key, cfgString value);
void cfg_set_float(cfgKey key, cfgFloat value);


/** Functions for querying the currently loaded option set.
 */
int cfg_option_count();
cfgOption const *cfg_option(int i);


/** Used by FLOAT options on C++/c89 to convert a float constant into its bit
 *  pattern expressed as an int. (This is to initialize the float member of a
 *  union correctly with out being able to specify it directly like C99 can.)
 */
cfgInt cfg_float2bitpattern(cfgFloat f);

cfgEXTERN_C_END_M

#endif/*cfgCONFIG_H*/
