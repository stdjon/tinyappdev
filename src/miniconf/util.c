#include "miniconf/config.h"
#include "miniconf/util.h"

static cfgString cfg_value_type_names_S[] =
{
    "<nonexistent>",
    "switch",
    "integer",
    "string",
    "real",
};


void cfg_report(cfgReport_f callback, void *param)
{
    int count = cfg_option_count();
    for(int i = 0; i < count; i++)
    {
        if( !callback(cfg_option(i), param) )
        {
            break;
        }
    }
}


/* use default if either param or param->NAME_ is zero... */
#define cfgGET_PARAM_M(NAME_, DEFAULT_) \
    (param ? (param->NAME_ ? param->NAME_ : DEFAULT_) : DEFAULT_)

cfgBool cfg_default_report(const cfgOption *opt, void *ptr) {
    cfgASSERT_M(opt);

    cfgDefaultReportOptions *param = (cfgDefaultReportOptions*)ptr;

    const char *name = opt->name;
    const char *abbr = opt->abbrev;
    const char *type = cfg_value_type_name(opt->value.type);
    const char *desc = opt->desc;

    if(!desc) { desc = "<undocumented>"; }

    size_t len = strlen(name) + (abbr ? strlen(abbr) : 0);
    if(abbr) { len += 3; }

    /* Calculate number of tab chars to print between option and its description.
     * (Be careful modifying this code - it seems quite sensitive to signed vs.
     * unsigned and integer overflow issues).
     */
    const int DESC_LEN = cfgGET_PARAM_M(indent, 8);
    const int TAB_LEN = cfgGET_PARAM_M(tabwidth, 8);
    int pad_len = (DESC_LEN - len);
    size_t tabs = (pad_len > 0) ? pad_len / TAB_LEN : 0;

    if(abbr) { printf( "  -%s (-%s):", name, abbr); }
    else { printf( "  -%s:", name); }

    if(tabs) {
        for(size_t i = 0; i < tabs; i++) { putchar('\t'); }
    } else {
        // At some point, give up on indentation, and just put a space between
        // the option and it's description.
        putchar(' ');
    }
    printf("%s [%s].\n", desc, type);

    return cfg_true;
}


cfgString cfg_value_type_name(cfgValueType type)
{
    if( (type < cfgVT_NOEXIST) || (type >= cfgVT_COUNT))
    {
        return 0;
    }

    return cfg_value_type_names_S[type - cfgVT_NOEXIST];
}


