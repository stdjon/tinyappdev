#include "miniconf/config.h"


/* In the interests of keeping this simple, there is considerable scope for
 * optimization (if that becomes necessary).
 * - The total number of options is limited to an arbitrary constant.
 * - Options are stored in an unsorted array - lookup is O(n).
 * - All names and abbreviations are allocated individually (even for single bytes!)
 *
 * As this is "only" the configuration system, and not a core component, and as
 * long as configuring the client app is "quick enough", ease of maintenance
 * should probably be the primary design consideration.
 */


/*----------------------------------------------------------------------------*/
/* sanity checks */

/* check we can store pointers */
cfgBTASSERT_M(sizeof(cfgInt) >= sizeof(void*), cfgInt_is_too_small);

/* basic sanity checks for cfgValueType types */
cfgBTASSERT_M(cfgVT_NOEXIST == cfgVT_TYPE_MASK, cfgVT_NOEXIST_doesnt_equal_cfgVT_TYPE_MASK);
cfgBTASSERT_M(cfgVT_SWITCH < cfgVT_COUNT, cfgVT_COUNT_too_small_for_cfgVT_SWITCH);
cfgBTASSERT_M(cfgVT_INT < cfgVT_COUNT, cfgVT_COUNT_too_small_for_cfgVT_INT);
cfgBTASSERT_M(cfgVT_STRING < cfgVT_COUNT, cfgVT_COUNT_too_small_for_cfgVT_STRING);
cfgBTASSERT_M(cfgVT_FLOAT < cfgVT_COUNT, cfgVT_COUNT_too_small_for_cfgVT_FLOAT);
cfgBTASSERT_M(cfgVT_COUNT <= cfgVT_NOEXIST, cfgVT_COUNT_too_large);

/* basic sanity checks for cfgValueType flags */
cfgBTASSERT_M(!(cfgVT_NOEXIST & cfgVT_FLAG_MASK), cfgVT_NOEXIST_in_flag_mask);
cfgBTASSERT_M(cfgVT_INPUT & cfgVT_FLAG_MASK, cfgVT_INPUT_not_in_flag_mask);
cfgBTASSERT_M(!(cfgVT_INPUT & cfgVT_TYPE_MASK), cfgVT_INPUT_in_type_mask);


/*----------------------------------------------------------------------------*/
/* Private API */

/* type / flag manipulation */
#define cfgVALUE_TYPE(opt_) ( ((opt_)->value.type) & cfgVT_TYPE_MASK )
#define cfgSET_FLAG(opt_, flag_) ( ((opt_)->value.type) |= flag_ )


typedef enum cfgReadArgsState
{
    cfgRAS_NORMAL,
    cfgRAS_CAPTURE,

} cfgReadArgsState;


/* stored argc/argv pair */
typedef struct cfgArgBlock
{
    int c;
    char **v;

} cfgArgBlock;

static cfgOption cfg_option_S[cfgMAX_OPTION_COUNT] = {{0}};
static int cfg_option_count_S = 0;
static cfgArgBlock cfg_block_S[cfgMAX_ARG_BLOCK_COUNT] = {{0}};
static int cfg_block_count_S = 0;


/* NB: if 0 is passed in, no allocation occurs. This is by design. */
static cfgMutableString cfg_copy_string(cfgString string)
{
    cfgMutableString result = 0;

    if(string)
    {
        cfgSize size = cfg_strlen(string) + 1;

        cfgMutableString buffer = cfgALLOC_M(size, cfgHINT_M("config key"));

        if(buffer)
        {
            cfg_strncpy(buffer, string, size);
            result = buffer;
        }
    }

    return result;
}


static cfgOption *cfg_find(cfgKey key)
{
    cfgASSERT_M(key);

    for(int i = 0; i < cfg_option_count_S; i++)
    {
        cfgOption *opt = &cfg_option_S[i];

        if((0 == cfg_strcmp(key, opt->name)) ||
            ( opt->abbrev && ( 0 == cfg_strcmp(key, opt->abbrev)) ))
        {
            return opt;
        }
    }

    return 0;
}


static cfgBool cfg_store(cfgOption const *option)
{
    cfgASSERT_M(cfg_option_count_S < cfgMAX_OPTION_COUNT);
    cfgASSERT_M(option->name);
    cfgASSERT_M(cfgVT_NOEXIST != cfgVALUE_TYPE(option));

    cfgASSERT_M(0 == cfg_find(option->name));

    cfgOption *new_option = &cfg_option_S[cfg_option_count_S];
    cfg_option_count_S++;

    new_option->name = cfg_copy_string(option->name);
    new_option->abbrev = cfg_copy_string(option->abbrev);
    new_option->desc = cfg_copy_string(option->desc);
    new_option->value = option->value;

    if(cfgVT_STRING == cfgVALUE_TYPE(option))
    {
        new_option->value.u.s = cfg_copy_string(option->value.u.s);
    }

    if( new_option->name &&
        (!new_option->abbrev == !option->abbrev) )
    {
        return cfg_true;
    }

    return cfg_false;
}


static cfgBool cfg_switch_off(cfgKey key, cfgReadArgsState *state)
{
    cfgASSERT_M(key);
    cfgASSERT_M(state);

    cfgOption *opt = cfg_find(key);

    if( (0 == opt) || /* no option */
        (cfgVT_SWITCH != cfgVALUE_TYPE(opt)) )
    {
        return cfg_false; 
    };

    /* turn the switch off */
    opt->value.u.b = cfg_false;
    cfgSET_FLAG(opt, cfgVT_INPUT);

    return cfg_true;
}



static void cfg_dispose_options()
{
    for(cfgSize i = 0; i < cfgMAX_OPTION_COUNT; i++)
    {
        cfgOption *opt = &cfg_option_S[i];

        if(opt->name) { cfgFREE_M((cfgMutableString)opt->name); }
        if(opt->abbrev) { cfgFREE_M((cfgMutableString)opt->abbrev); }
        if(opt->desc) { cfgFREE_M((cfgMutableString)opt->desc); }
        if(cfgVT_STRING == cfgVALUE_TYPE(opt))
        {
            cfgFREE_M((cfgMutableString)opt->value.u.s);
        }

        opt->name = opt->abbrev = opt->value.u.s = 0;
    }
}


static cfgBool cfg_check_options(
    cfgKey key, cfgReadArgsState *state, cfgString *set)
{
    cfgASSERT_M(key);
    cfgASSERT_M(state);
    cfgASSERT_M(set);

    cfgOption *opt = cfg_find(key);

    if(0 == opt) { return cfg_false; } /* no option */

    if(cfgVT_SWITCH == cfgVALUE_TYPE(opt))
    {
        /* turn the switch on */
        opt->value.u.b = cfg_true;
        cfgSET_FLAG(opt, cfgVT_INPUT);
        return cfg_true;
    }
    else
    {
        *state = cfgRAS_CAPTURE;
        *set = key;
    }

    return cfg_true;
}


/*----------------------------------------------------------------------------*/
/* Arg Blocks */

static void cfg_new_block(int size)
{
    cfgASSERT_M(cfg_block_count_S < cfgMAX_ARG_BLOCK_COUNT);
    cfgArgBlock *block = &cfg_block_S[cfg_block_count_S];
    cfgASSERT_M(block);
    block->v = cfgALLOC_M(sizeof(block->v) * size, "Arg Block");
    cfgASSERT_M(block->v);
    block->c = 0;
}


static void cfg_finish_block(int *ac, char ***av)
{
    cfgArgBlock *block = &cfg_block_S[cfg_block_count_S];
    cfgASSERT_M(block);
    *ac = block->c;
    *av = block->v;
    cfg_block_count_S++;
}


static cfgBool cfg_copy_to_block(cfgString s)
{
    cfgASSERT_M(cfg_block_count_S >= 0);
    cfgASSERT_M(cfg_block_count_S < cfgMAX_ARG_BLOCK_COUNT);
    cfgArgBlock *block = &cfg_block_S[cfg_block_count_S];
    cfgASSERT_M(block);
    cfgASSERT_M(block->c < cfgMAX_ARG_COUNT);

    block->v[block->c++] = cfg_copy_string(s);

    return cfg_true;
}


void cfg_dispose_blocks()
{
    for(int i = 0; i < cfg_block_count_S; i++)
    {
        cfgArgBlock *block = &cfg_block_S[i];
        cfgASSERT_M(block);

        for(int j = 0; j < block->c; j++)
        {
            cfgFREE_M(block->v[j]);
        }
        cfgFREE_M(block->v);
    }
    cfg_block_count_S = 0;
}


/*----------------------------------------------------------------------------*/

static cfgBool cfg_copy_to_option(cfgString s, cfgReadArgsState *state, cfgString *set)
{
    cfgASSERT_M(s);
    cfgASSERT_M(state);
    cfgASSERT_M(set);

    cfgOption *opt = cfg_find(*set);

    *set = 0;
    *state = cfgRAS_NORMAL;

    if(0 == opt) { return cfg_false; } /* no option */

    switch(cfgVALUE_TYPE(opt))
    {
        case cfgVT_INT: { opt->value.u.i = cfg_atoi(s); break; }
        case cfgVT_STRING: { opt->value.u.s = cfg_copy_string(s); break; }
        case cfgVT_FLOAT: { opt->value.u.f = cfg_atof(s); break; }
        default: { cfgASSERT_M(!"bad value type"); }
    }
    cfgSET_FLAG(opt, cfgVT_INPUT);

    return cfg_true;
}


static cfgValueType cfg_flags(cfgKey key)
{
    cfgASSERT_M(key);

    cfgOption *opt = cfg_find(key);

    return opt ?
        (opt->value.type & cfgVT_FLAG_MASK) :
        cfgVT_NOEXIST;
}


/*----------------------------------------------------------------------------*/
/* Public API */

static cfgBool cfg_initialized = cfg_false;

cfgBool cfg_init()
{
    cfgASSERT_M(!cfg_initialized);
    return cfg_initialized = cfg_true;
}


void cfg_exit()
{
    cfgASSERT_M(cfg_initialized);

    cfg_dispose_options();
    cfg_dispose_blocks();
    cfg_initialized = cfg_false;
}


cfgBool cfg_add_options(cfgOption const *option)
{
    cfgASSERT_M(option);

    cfgBool result = cfg_true;

    while(result && option->name)
    {
        result = cfg_store(option);
        option++;
    }

    return result;
}


cfgBool cfg_read_args(int *argc, char ***argv)
{
    cfgBool result = cfg_false;
    cfgASSERT_M(argc && argv); /** bad input data */

    int ac = *argc;
    char **av = *argv;
    cfgASSERT_M(ac < cfgMAX_ARG_COUNT); /* too many args */

    cfgReadArgsState state = cfgRAS_NORMAL;
    cfgString current_option = 0;

    cfg_new_block(ac);

    /* copy argv[0] to output argv and ignore it ever after */
    cfg_copy_to_block(av[0]);

    for(int i = 1; i < ac; i++)
    {
        cfgString arg = av[i];
        cfgChar arg_first = arg[0];
        cfgString arg_rest = arg + 1;

        switch(state)
        {
            case cfgRAS_NORMAL:
            {
                switch(arg_first)
                {
                    case '+':
                    {
                        if(!cfg_switch_off(arg_rest, &state))
                        {
                            goto cfg_read_args_finish;
                        }
                        break;
                    }
                    case '-':
                    {
                        if(!cfg_check_options(arg_rest, &state, &current_option))
                        {
                            if(!cfg_copy_to_block(arg))
                            {
                                goto cfg_read_args_finish;
                            }
                        }
                        break;
                    }
                    default:
                    {
                        if(!cfg_copy_to_block(arg))
                        {
                            goto cfg_read_args_finish;
                        }
                        break;
                    }
                }
                break;
            }

            case cfgRAS_CAPTURE:
            {
                if(!cfg_copy_to_option(arg, &state, &current_option))
                {
                    if(!cfg_copy_to_block(arg))
                    {
                        goto cfg_read_args_finish;
                    }
                }
                break;
            }
        }
    }

    result = cfg_true;

cfg_read_args_finish:
    cfg_finish_block(argc, argv);
    return result;
}


cfgBool cfg_read_file(cfgString path)
{
    cfgFILE *file = cfg_fopen(path, "r");
    cfgBool result = cfg_false;

    if(file)
    {
        enum { SIZE = 0x1000, };
        cfgMutableString buffer = cfgALLOC_M(SIZE, cfgHINT_M("cfg read file"));

        if(buffer)
        {
            while(buffer == cfg_fgets(buffer, SIZE, file))
            {
                cfgMutableString nl = strchr(buffer, '\n');
                cfgASSERT_M(nl);
                *nl = 0;

                switch(buffer[0])
                {
                    case '-': { cfg_set_switch(&buffer[1], cfg_true); break; }
                    case '+': { cfg_set_switch(&buffer[1], cfg_false); break; }
                    case '#': { break; } /* comment */
                    default:
                    {
                        cfgString equals = strchr(buffer, '=');

                        if(!equals) { goto done; }

                        cfgChar tmp[256] = {0}; /* FIXME */
                        strncpy(tmp, buffer, (equals - buffer));

                        cfgString value = equals + 1;

                        cfgValueType type = cfg_type(tmp);

                        switch(type)
                        {
                            case cfgVT_INT:
                            {
                                cfg_set_int(tmp, cfg_atoi(value));
                                break;
                            }

                            case cfgVT_STRING:
                            {
                                cfg_set_string(tmp, value);
                                break;
                            }

                            case cfgVT_FLOAT:
                            {
                                cfg_set_float(tmp, cfg_atof(value));
                                break;
                            }

                            default:
                            {
                                cfg_printf("unknown option '%s'\n", tmp);
                                break;
                            }
                        }

                        break;
                    }
                }
            }

            result = cfg_true;
done:
            cfg_fclose(file);
            cfgFREE_M(buffer);
        }
    }

    return result;
}


cfgValueType cfg_type(cfgKey key)
{
    cfgASSERT_M(key);

    cfgOption *opt = cfg_find(key);

    return opt ? cfgVALUE_TYPE(opt) : cfgVT_NOEXIST;
}


cfgBool cfg_input(cfgKey key)
{
    cfgASSERT_M(key);

    return !!(cfg_flags(key) & cfgVT_INPUT);
}


cfgBool cfg_switch(cfgKey key)
{
    cfgASSERT_M(key);

    cfgOption *opt = cfg_find(key);
    cfgASSERT_M(opt);
    cfgASSERT_M(cfgVT_SWITCH == cfgVALUE_TYPE(opt));

    return opt->value.u.b;
}


cfgInt cfg_int(cfgKey key)
{
    cfgASSERT_M(key);

    cfgOption *opt = cfg_find(key);
    cfgASSERT_M(opt);
    cfgASSERT_M(cfgVT_INT == cfgVALUE_TYPE(opt));

    return opt->value.u.i;
}


cfgString cfg_string(cfgKey key)
{
    cfgASSERT_M(key);

    cfgOption *opt = cfg_find(key);
    cfgASSERT_M(opt);
    cfgASSERT_M(cfgVT_STRING == cfgVALUE_TYPE(opt));

    return opt->value.u.s;
}


cfgFloat cfg_float(cfgKey key)
{
    cfgASSERT_M(key);

    cfgOption *opt = cfg_find(key);
    cfgASSERT_M(opt);
    cfgASSERT_M(cfgVT_FLOAT == cfgVALUE_TYPE(opt));

    return opt->value.u.f;
}


void cfg_set_switch(cfgKey key, cfgBool value)
{
    cfgASSERT_M(key);

    cfgOption *opt = cfg_find(key);
    cfgASSERT_M(opt);
    cfgASSERT_M(cfgVT_SWITCH == cfgVALUE_TYPE(opt));

    opt->value.u.b = value;
    cfgSET_FLAG(opt, cfgVT_INPUT);
}


void cfg_set_int(cfgKey key, cfgInt value)
{
    cfgASSERT_M(key);

    cfgOption *opt = cfg_find(key);
    cfgASSERT_M(opt);
    cfgASSERT_M(cfgVT_INT == cfgVALUE_TYPE(opt));

    opt->value.u.i = value;
    cfgSET_FLAG(opt, cfgVT_INPUT);
}


void cfg_set_string(cfgKey key, cfgString value)
{
    cfgASSERT_M(key);

    cfgOption *opt = cfg_find(key);
    cfgASSERT_M(opt);
    cfgASSERT_M(cfgVT_STRING == cfgVALUE_TYPE(opt));

    cfgString *string = &opt->value.u.s;

    if(*string) { cfgFREE_M(*(cfgMutableString*)string); }

    *string = cfg_copy_string(value);
    cfgSET_FLAG(opt, cfgVT_INPUT);
}


void cfg_set_float(cfgKey key, cfgFloat value)
{
    cfgASSERT_M(key);

    cfgOption *opt = cfg_find(key);
    cfgASSERT_M(opt);
    cfgASSERT_M(cfgVT_FLOAT == cfgVALUE_TYPE(opt));

    opt->value.u.f = value;
    cfgSET_FLAG(opt, cfgVT_INPUT);
}


int cfg_option_count()
{
    return cfg_option_count_S;
}


cfgOption const *cfg_option(int i)
{
    return &cfg_option_S[i];
}


cfgInt cfg_float2bitpattern(cfgFloat f)
{
    /* dereferencing type-punned pointer will break strict-aliasing rules [-Werror=strict-aliasing] */
    /* return *((cfgInt*)&f); */

    union Float2Int
    {
        cfgFloat in;
        cfgInt out;
    } x;

    x.in = f;
    return x.out;
}


