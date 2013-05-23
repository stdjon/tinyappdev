#include "tests/pml.h"
#include "pml/malloc.h"

#include <malloc.h>

// NB: this is C99 code, we can use // line comment format

//------------------------------------------------------------------------------
// instance counters

struct Counters {

    int mallocs;
    int callocs;
    int reallocs;
    int frees;
    int news;
    int deletes;
    int newas;
    int deleteas;
    int hook_allocs;
    int hook_callocs;
    int hook_reallocs;
    int hook_frees;
    int objects;

} counters;


void reset(struct Counters *k) {
    k->mallocs = 0;
    k->callocs = 0;
    k->reallocs = 0;
    k->frees = 0;
    k->news = 0;
    k->deletes = 0;
    k->newas = 0;
    k->deleteas = 0;
    k->hook_allocs = 0;
    k->hook_callocs = 0;
    k->hook_reallocs = 0;
    k->hook_frees = 0;
    k->objects = 0;
}


//------------------------------------------------------------------------------
// pml hooks

static void c_debug_hook(const PmlDebugHookInfo *info) {

    switch(info->type) {
        case pml_MALLOC: { counters.mallocs++; break; }
        case pml_CALLOC: { counters.callocs++; break; }
        case pml_REALLOC: { counters.reallocs++; break; }
        case pml_FREE: { counters.frees++; break; }
        case pml_NEW: { counters.news++; break; }
        case pml_DELETE: { counters.deletes++; break; }
        case pml_NEWA: { counters.newas++; break; }
        case pml_DELETEA: { counters.deleteas++; break; }
    }

    if(info->hint) {
        TFR_trace(2, "debug: %s\n", info->hint);
    }

    TFR_trace(5, "test_debug_hook: type: %u count: %u size: %u ptr: %p in: %p\n"
        "                 alloc: %p\n"
        "                 hint: %s\n",
        info->type, info->count, info->size,
        info->ptr, info->in,
        info->alloc, info->hint);
}


static void c_assert_hook(const PmlAssertHookInfo *info) {
    TFR_trace(1, "test_assert_hook: %s(%u) \"%s\" failed.\n",
        info->file, info->line, info->expr);
}


static void *c_malloc_hook(size_t size, PmlAllocator *a, PmlHint h) {

    counters.hook_allocs++;
    return malloc(size);
}


static void c_free_hook(void *ptr, PmlAllocator *a, PmlHint h) {

    counters.hook_frees++;
    free(ptr);
}


static void *c_calloc_hook(size_t count, size_t size, PmlAllocator *a, PmlHint h) {

    counters.hook_callocs++;
    return calloc(count, size);
}


static void *c_realloc_hook(void *ptr, size_t size, PmlAllocator *a, PmlHint h) {

    counters.hook_reallocs++;
    return realloc(ptr, size);
}


//------------------------------------------------------------------------------
// open/close for all misc tests...

TFR_Bool c_pml_open() {

    return
        pml_set_debug_hook(c_debug_hook) &&
        pml_set_assert_hook(c_assert_hook) &&
        pml_set_malloc_hook(c_malloc_hook) &&
        pml_set_free_hook(c_free_hook) &&
        pml_set_calloc_hook(c_calloc_hook) &&
        pml_set_realloc_hook(c_realloc_hook);
}

void c_pml_close() {
}


//------------------------------------------------------------------------------

TFR_Bool c_test_malloc() {

    bool result = false;

    reset(&counters);

    void *ptr = pml_malloc(1024);

    if( ptr &&
        (1 == counters.mallocs) &&
        (1 == counters.hook_allocs) ) {

        pml_free(ptr);

        if( (1 == counters.frees) &&
            (1 == counters.hook_frees) ) {

            result = true;
        }
    }

    return result;
}



#ifdef PML_HAS_C11
TFR_Bool c_test_malloc_hint() {

    bool result = false;

    reset(&counters);

    void *ptr = pml_malloc(1024, PML_HINT("this works?"));

    if( ptr &&
        (1 == counters.mallocs) &&
        (1 == counters.hook_allocs) ) {

        pml_free(ptr, PML_HINT("this works?"));

        if( (1 == counters.frees) &&
            (1 == counters.hook_frees) ) {

            result = true;
        }
    }

    return result;
}
#endif//PML_HAS_C11
//------------------------------------------------------------------------------
size_t test = 0;

void *c_malloc_alloc(size_t size, PmlAllocator *a, PmlHint h) {

    if(h) { TFR_trace(3, "hint: %s\n", h); }

    test++;
    return malloc(size);
}


void c_free_alloc(void *ptr, PmlAllocator *a, PmlHint h) {

    if(h) { TFR_trace(3, "hint: %s\n", h); }

    test--;
    free(ptr);
}


TFR_Bool c_test_malloc_alloc() {

    if(TFR_check(4, 0 == test)) {

        PmlAllocator alloc;
        pml_init_allocator(&alloc, c_malloc_alloc, c_free_alloc);

        void *mymem = pml_malloc(1024, &alloc);//, PML_HINT("mymem"));

        if( TFR_check(4, !!mymem) &
            TFR_check(4, 1 == test) ) {

            pml_free(mymem, &alloc);//, PML_HINT("mymem"));

            if(TFR_check(4, 0 == test)) {
                return TFR_true;
            }
        }
    }

    return TFR_false;
}


//------------------------------------------------------------------------------

TFR_Bool c_test_emulate_calloc() {

    TFR_Bool result = TFR_true;

    if(TFR_check(4, 0 == test)) {

        PmlAllocator alloc;
        pml_init_allocator(&alloc,
            c_malloc_alloc, c_free_alloc, pml_emulate_calloc);

        int *mymem = pml_calloc(sizeof(int), 256, &alloc);

        if( TFR_check(4, !!mymem) &
            TFR_check(4, 1 == test) ) {

            for(int i = 0; i < 256; i++) {
                if(mymem[i] != 0) {
                    result = TFR_false;
                }
            }

            pml_free(mymem, &alloc);

            if(TFR_check(4, 0 == test)) {
                return result;
            }
        }
    }

    return TFR_false;
}


//------------------------------------------------------------------------------

TFR_Bool c_test_emulate_realloc() {

    TFR_Bool result = TFR_true;

    if(TFR_check(4, 0 == test)) {

        PmlAllocator alloc;
        pml_init_allocator(&alloc,
            c_malloc_alloc, c_free_alloc, 0, pml_emulate_realloc);

        int *mymem = pml_malloc(3 * sizeof(int), &alloc);
        mymem[0] = 1;
        mymem[1] = 2;
        mymem[2] = 3;

        if( TFR_check(4, !!mymem) &
            TFR_check(4, 1 == test) ) {

            int *mymem2 = pml_realloc(mymem, 4 * sizeof(int), &alloc);

            if( TFR_check(4, !!mymem2) &
                TFR_check(4, 1 == test) ) {

                if( TFR_check(4, mymem2[0] != 1) |
                    TFR_check(4, mymem2[1] != 2) |
                    TFR_check(4, mymem2[2] != 3) ) {

                    result = TFR_false;
                }
            }

            pml_free(mymem2, &alloc);

            if(TFR_check(4, 0 == test)) {
                return result;
            }
        }
    }

    return TFR_false;
}


void c_declare_pml_tests() {

    TFR_SUITE_DECLARE_M("pml::c", c_pml_open, c_pml_close);
    TFR_SUITE_ADD_M(c_test_malloc);
#ifdef PML_HAS_C11
    TFR_SUITE_ADD_M(c_test_malloc_hint);
#endif//PML_HAS_C11
    TFR_SUITE_ADD_M(c_test_malloc_alloc);
    TFR_SUITE_ADD_M(c_test_emulate_calloc);
    TFR_SUITE_ADD_M(c_test_emulate_realloc);
}


