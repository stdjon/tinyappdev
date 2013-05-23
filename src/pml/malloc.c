#include "pml/malloc.h"

#include <stdint.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>

/*----------------------------------------------------------------------------*/
/* Default malloc/free impls */

#ifdef PML_ASSERT_HOOK_S
static void pml_assert_(const PmlAssertHookInfo *info) {

    printf("*** PML assertion failed:\n\t%s(%u) \"%s\".\n",
        info->file, (unsigned int)info->line, info->expr);

    /* We'll continue running, but who knows what will happen... */
}
#endif/*PML_ASSERT_HOOK_S*/


static void *pml_malloc_(size_t size,
    PML_TYPE(Allocator) *a, PML_TYPE(Hint) h) {

    return malloc(size);
}


static void pml_free_(void *ptr,
    PML_TYPE(Allocator) *a, PML_TYPE(Hint) h) {

    free(ptr);
}


static void *pml_calloc_(size_t count, size_t size,
    PML_TYPE(Allocator) *a, PML_TYPE(Hint) h) {

    return calloc(count, size);
}


static void *pml_realloc_(void *ptr, size_t size,
    PML_TYPE(Allocator) *a, PML_TYPE(Hint) h) {

    return realloc(ptr, size);
}


/*----------------------------------------------------------------------------*/
/* Hooks */

static PML_TYPE(MallocHook) s_pml_malloc_hook = pml_malloc_;
static PML_TYPE(FreeHook) s_pml_free_hook = pml_free_;
static PML_TYPE(CallocHook) s_pml_calloc_hook = pml_calloc_;
static PML_TYPE(ReallocHook) s_pml_realloc_hook = pml_realloc_;
static PML_TYPE(DebugHook) s_pml_debug_hook = 0;
static PML_TYPE(AssertHook) s_pml_assert_hook = pml_assert_;


bool PML_APINAME(set_debug_hook)(PML_TYPE(DebugHook) hook) {

    s_pml_debug_hook = hook;
    return true;
}


bool PML_APINAME(set_assert_hook)(PML_TYPE(AssertHook) hook) {

    s_pml_assert_hook = hook;
    return true;
}


bool PML_APINAME(set_malloc_hook)(PML_TYPE(MallocHook) hook) {

    if(!hook) {
        hook = pml_malloc_;
    }
    PML_ASSERT(hook);
    s_pml_malloc_hook = hook;
    return true;
}


bool PML_APINAME(set_free_hook)(PML_TYPE(FreeHook) hook) {

    if(!hook) {
        hook = pml_free_;
    }
    PML_ASSERT(hook);
    s_pml_free_hook = hook;
    return true;
}


bool PML_APINAME(set_calloc_hook)(PML_TYPE(CallocHook) hook) {

    if(!hook) {
        hook = pml_calloc_;
    }
    PML_ASSERT(hook);
    s_pml_calloc_hook = hook;
    return true;
}


bool PML_APINAME(set_realloc_hook)(PML_TYPE(ReallocHook) hook) {

    if(!hook) {
        hook = pml_realloc_;
    }
    PML_ASSERT(hook);
    s_pml_realloc_hook = hook;
    return true;
}


/*----------------------------------------------------------------------------*/
/* Debug hook handler */

#ifdef PML_DEBUG_HOOK_S
void PML_APINAME(debug_hook)(PML_TYPE(DebugHookType) type,
    size_t count, size_t size, void *ptr, void *in,
    PML_TYPE(Allocator) *alloc, PML_TYPE(Hint) hint) {

    if(s_pml_debug_hook) {
        PML_TYPE(DebugHookInfo) info;
        info.type = type;
        info.count = count;
        info.size = size;
        info.ptr = ptr;
        info.in = in;
        info.alloc = alloc;
        info.hint = hint;
        s_pml_debug_hook(&info);
    }
}
#endif/*PML_DEBUG_HOOK_S*/


/*----------------------------------------------------------------------------*/
/* Assert hook handler */

#ifdef PML_DEBUG_HOOK_S
void PML_APINAME(assert_hook)(bool test, const char *expr,
    const char *file, size_t line) {

    if( !test &&
        s_pml_assert_hook ) {

        PML_TYPE(AssertHookInfo) info;
        info.expr = expr;
        info.file = file;
        info.line = line;
        s_pml_assert_hook(&info);
    }
}
#endif/*PML_DEBUG_HOOK_S*/


/*----------------------------------------------------------------------------*/
/* malloc() */

void *PML_APINAME(malloc)(size_t size,
    PML_TYPE(Allocator) *alloc, PML_TYPE(Hint) hint) {

    PML_TYPE(MallocHook) hook = alloc ? alloc->malloc : s_pml_malloc_hook;
    PML_ASSERT(hook);

    void *ptr = hook(size, alloc, hint);
    PML_DEBUG_HOOK(MALLOC, 0, size, ptr, 0, alloc, hint);

    return ptr;
}


/*----------------------------------------------------------------------------*/
/* free() */

void PML_APINAME(free)(void *ptr,
    PML_TYPE(Allocator) *alloc, PML_TYPE(Hint) hint) {

    PML_TYPE(FreeHook) hook = alloc ? alloc->free : s_pml_free_hook;
    PML_ASSERT(hook);

    hook(ptr, alloc, hint);
    PML_DEBUG_HOOK(FREE, 0, 0, ptr, 0, alloc, hint);
}


/*----------------------------------------------------------------------------*/
/* calloc() */

void *PML_APINAME(calloc)(size_t count, size_t size,
    PML_TYPE(Allocator) *alloc, PML_TYPE(Hint) hint) {

    PML_TYPE(CallocHook) hook = alloc ? alloc->calloc : s_pml_calloc_hook;
    PML_ASSERT(hook);

    void *ptr = hook(count, size, alloc, hint);
    PML_DEBUG_HOOK(CALLOC, count, size, ptr, 0, alloc, hint);

    return ptr;
}


/*----------------------------------------------------------------------------*/
/* realloc() */

void *PML_APINAME(realloc)(void *ptr, size_t size,
    PML_TYPE(Allocator) *alloc, PML_TYPE(Hint) hint) {

    PML_TYPE(ReallocHook) hook = alloc ? alloc->realloc : s_pml_realloc_hook;
    PML_ASSERT(hook);

    void *out = hook(ptr, size, alloc, hint);
    PML_DEBUG_HOOK(REALLOC, 0, size, out, ptr, alloc, hint);

    return out;
}


/*----------------------------------------------------------------------------*/
/* emulate_calloc() */

/** This is quite easy - malloc(count * size) then memset() to zero...
 */
void *PML_APINAME(emulate_calloc)(
    size_t count, size_t size,
    PML_TYPE(Allocator) *alloc, PML_TYPE(Hint) hint) {

    PML_TYPE(MallocHook) hook = alloc ? alloc->malloc : s_pml_malloc_hook;
    PML_ASSERT(hook);

    size_t bytes = count * size;
    PML_ASSERT(bytes >= (uint64_t)count * size); /* or we have an overflow */

    void *ptr = hook(bytes, alloc, hint);
    if(ptr) {
        memset(ptr, 0, bytes);
    }

    PML_DEBUG_HOOK(CALLOC, count, size, ptr, 0, alloc, hint);

    return ptr;
}


/*----------------------------------------------------------------------------*/
/* emulate_realloc() */

/** Without a portable means to query the size of an existing allocation returned
 *  from malloc(), our hands are tied - all we can do is use malloc() to create a
 *  new block, copy up to size from the old ptr, then free(ptr). We have to hope
 *  this doesn't cause an invalid read past the end of ptr, and we can't optimise
 *  shrinking the block by returning the existing ptr either. Anyone who cares
 *  strongly enough about this will provide their own realloc() implementation
 *  instead of using this one.
 */
void *PML_APINAME(emulate_realloc)(
    void *ptr, size_t size,
    PML_TYPE(Allocator) *alloc, PML_TYPE(Hint) hint) {

    PML_TYPE(MallocHook) malloc_hook = alloc ? alloc->malloc : s_pml_malloc_hook;
    PML_TYPE(FreeHook) free_hook = alloc ? alloc->free : s_pml_free_hook;
    PML_ASSERT(malloc_hook && free_hook);

    void *out = 0;

    if(size > 0) {
        out = malloc_hook(size, alloc, hint);
    }

    if(out) {
        memcpy(out, ptr, size);
    }

    if(ptr && out) {
        free_hook(ptr, alloc, hint);
    }

    PML_DEBUG_HOOK(REALLOC, 0, size, out, ptr, alloc, hint);

    return out;
}



