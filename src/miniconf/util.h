#ifndef cfgUTIL_H
#define cfgUTIL_H

/** \file
 *  \ingroup Config
 *
 *  Miniconf utilities.
 */

cfgEXTERN_C_START_M


/*----------------------------------------------------------------------------*/
/* Types */

/** Options for cfg_default_report()
 */
typedef struct cfgDefaultReportOptions {

    /** Desired indentation after option name.
     *  (Adjust until options line up satisfactorily.)
     */
    size_t indent; 

    /** width of the TAB (\t) character in spaces
     */
    size_t tabwidth;

} cfgDefaultReportOptions;


/** Signature of the cfg_report() callback.
 */
typedef cfgBool (*cfgReport_f)(cfgOption const*, void*);


/*----------------------------------------------------------------------------*/
/* Functions */

/** Calls passed in callback on each config option currently set. Can be used
 *  to dump config state.
 *
 *  callback: name of a function which can be called for each option.
 */
void cfg_report(cfgReport_f callback, void *param);


/** A default report handler for cfg_report()
 *  Param can be null or a pointer to a cfgDefaultReportOptions struct.
 */
cfgBool cfg_default_report(const cfgOption *opt, void *param);


/** return the type of an option as a string.
 */
cfgString cfg_value_type_name(cfgValueType type);




cfgEXTERN_C_END_M

#endif/*cfgUTIL_H*/
