#include "tests/pml.h"
#include "pml/malloc.h"

#include <malloc.h>


namespace tests {
namespace pml {

// instance counters
struct Counters {

    Counters() { reset(); }

    void reset() {
        mallocs = 0;
        callocs = 0;
        reallocs = 0;
        frees = 0;
        news = 0;
        deletes = 0;
        newas = 0;
        deleteas = 0;
        hook_allocs = 0;
        hook_frees = 0;
        objects = 0;
    }

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


struct Object {

    // for array alloc
    Object() {
        counters.objects++;
    }

    // for single alloc
    Object(int force_count, int a, int b, int c, int d) {
        counters.objects = force_count;
    }

#ifdef PML_HAS_CPP11
    // Ctor with more than 5 args (max for C++98)
    Object(int force_count, int a, int b, int c, int d, int e, int f, int g) {
        counters.objects = force_count;
    }
#endif//PML_HAS_CPP11

    ~Object() {
        counters.objects--;
    }
};


//------------------------------------------------------------------------------
// pml hooks

void test_debug_hook(const ::pml::DebugHookInfo *info) {

    switch(info->type) {
        case ::pml::MALLOC: { counters.mallocs++; break; }
        case ::pml::CALLOC: { counters.callocs++; break; }
        case ::pml::REALLOC: { counters.reallocs++; break; }
        case ::pml::FREE: { counters.frees++; break; }
        case ::pml::NEW: { counters.news++; break; }
        case ::pml::DELETE: { counters.deletes++; break; }
        case ::pml::NEWA: { counters.newas++; break; }
        case ::pml::DELETEA: { counters.deleteas++; break; }
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


void test_assert_hook(const ::pml::AssertHookInfo *info) {
    TFR_trace(1, "test_assert_hook: %s(%u) \"%s\" failed.\n",
        info->file, info->line, info->expr);
}


void *test_malloc_hook(size_t size, ::pml::Allocator*, ::pml::Hint) {

    counters.hook_allocs++;
    return ::malloc(size);
}


void test_free_hook(void *ptr, ::pml::Allocator*, ::pml::Hint) {

    counters.hook_frees++;
    ::free(ptr);
}


void *test_calloc_hook(size_t count, size_t size, ::pml::Allocator*, ::pml::Hint) {

    counters.hook_callocs++;
    return ::calloc(count, size);
}


void *test_realloc_hook(void *ptr, size_t size, ::pml::Allocator*, ::pml::Hint) {

    counters.hook_reallocs++;
    return ::realloc(ptr, size);
}


//------------------------------------------------------------------------------
// open/close for all pml tests...

TFR_Bool pml_open() {

    return
         ::pml_set_debug_hook(test_debug_hook) &&
         ::pml_set_malloc_hook(test_malloc_hook) &&
         ::pml_set_free_hook(test_free_hook) &&
         ::pml_set_calloc_hook(test_calloc_hook) &&
         ::pml_set_realloc_hook(test_realloc_hook);
}

void pml_close() {
}


//------------------------------------------------------------------------------

TFR_Bool test_malloc() {

    bool result = false;

    counters.reset();

    void *ptr = ::pml_malloc(1024);

    if( ptr &&
        (1 == counters.mallocs) &&
        (1 == counters.hook_allocs) ) {

        ::pml_free(ptr);

        if( (1 == counters.frees) &&
            (1 == counters.hook_frees) ) {

            result = true;
        }
    }

    return result;
}


//------------------------------------------------------------------------------

TFR_Bool test_new() {

    bool result = false;

    counters.reset();

    Object *ptr = ::pml_new<Object>()(1024, 2, 3, 4, 5); // new Object(1024, 2, 3, 4, 5);

    if( ptr &&
        (1024 == counters.objects) &&
        (1 == counters.news) &&
        (1 == counters.mallocs) &&
        (1 == counters.hook_allocs) ) {

        ::pml_delete()(ptr); // delete ptr;

        if( (1023 == counters.objects) &&
            (1 == counters.deletes) &&
            (1 == counters.frees) &&
            (1 == counters.hook_frees) ) {

            Object *ptr = ::pml_new<Object>()(); // new Object();

            if( ptr &&
                (1024 == counters.objects) &&
                (2 == counters.news) &&
                (2 == counters.mallocs) &&
                (2 == counters.hook_allocs) ) {

                ::pml_delete()(ptr); // delete ptr;

                if( (1023 == counters.objects) &&
                    (2 == counters.deletes) &&
                    (2 == counters.frees) &&
                    (2 == counters.hook_frees) ) {

                    result = true;
                }
            }
        }
    }

    return result;
}


#ifdef PML_EMPTY_NEW_S
TFR_Bool test_empty_new() {

    counters.reset();

    const Object *ptr = ::pml_new<Object>(); // new Object;

    if( ptr &&
        (1 == counters.objects) &&
        (1 == counters.news) &&
        (1 == counters.mallocs) &&
        (1 == counters.hook_allocs) ) {

        ::pml_delete()(ptr); // delete ptr;

        if( (0 == counters.objects) &&
            (1 == counters.deletes) &&
            (1 == counters.frees) &&
            (1 == counters.hook_frees) ) {

            return TFR_true;
        }
    }

    return TFR_false;
}
#endif//PML_EMPTY_NEW_S



#ifdef PML_HAS_CPP11
TFR_Bool test_new_8args() {

    bool result = false;

    counters.reset();

    Object *ptr = ::pml_new<Object>()(1024, 2, 3, 4, 5, 6, 7, 8); // new Object(1024, 2, 3, 4, 5, 6, 7, 8);

    if( ptr &&
        (1024 == counters.objects) &&
        (1 == counters.news) &&
        (1 == counters.mallocs) &&
        (1 == counters.hook_allocs) ) {

        ::pml_delete()(ptr); // delete ptr;

        if( (1023 == counters.objects) &&
            (1 == counters.deletes) &&
            (1 == counters.frees) &&
            (1 == counters.hook_frees) ) {

            result = true;
        }
    }

    return result;
}
#endif//PML_HAS_CPP11


//------------------------------------------------------------------------------

TFR_Bool test_newa() {

    bool result = false;

    counters.reset();

    Object *ptr = ::pml_newa<Object>(24); // new Object[24];

    if( ptr &&
        (24 == counters.objects) &&
        (1 == counters.newas) &&
        (1 == counters.mallocs) &&
        (1 == counters.hook_allocs) ) {

        ::pml_deletea(ptr); // delete[] ptr

        if( (0 == counters.objects) &&
            (1 == counters.deleteas) &&
            (1 == counters.frees) &&
            (1 == counters.frees) ) {

            result = true;
        }
    }

    return result;
}


TFR_Bool test_new_array() {

    bool result = false;

    counters.reset();

    Object *ptr = ::pml_new<Object>()[24]; // new Object[24];

    if( ptr &&
        (24 == counters.objects) &&
        (1 == counters.newas) &&
        (1 == counters.mallocs) &&
        (1 == counters.hook_allocs) ) {

        ::pml_delete()[ptr]; // delete[] ptr;

        if( (0 == counters.objects) &&
            (1 == counters.deleteas) &&
            (1 == counters.frees) &&
            (1 == counters.frees) ) {

            result = true;
        }
    }

    return result;
}


//------------------------------------------------------------------------------

struct MyAllocator: ::pml::IAllocator {

    int allocs;

    MyAllocator(): allocs(0) {}

    virtual void *malloc(size_t size, ::pml::Hint = 0) {
        allocs++;
        return ::malloc(size);
    }

    virtual void free(void *ptr, ::pml::Hint = 0) {
        allocs--;
        return ::free(ptr);
    }
};


TFR_Bool test_iallocator() {

    MyAllocator s;

    if(TFR_check(4, 0 == s.allocs)) {
        void *p = s.malloc(100);

        if(TFR_check(4, 1 == s.allocs)) {
            s.free(p);

            if(TFR_check(4, 0 == s.allocs)) {
                p = ::pml_malloc(100, &s);

                if(TFR_check(4, 1 == s.allocs)) {
                    ::pml_free(p, &s);

                    if(TFR_check(4, 0 == s.allocs)) {
                        return TFR_true;
                    }
                }
            }
        }
    }

    return TFR_false;
}


TFR_Bool test_iallocator2() {

    MyAllocator s;

    if(TFR_check(4, 0 == s.allocs)) {
        Object *o = pml_new<Object>(&s)(100, 2, 3, 4, 5); // new Object(100, 2, 3, 4, 5);

        if( TFR_check(4, 1 == s.allocs) &&
            TFR_check(4, 100 == counters.objects) ) {

            pml_delete(&s)(o); // delete o;

            if( TFR_check(4, 0 == s.allocs) &&
                TFR_check(4, 99 == counters.objects) ) {

                counters.reset();

                o = pml_new<Object>(&s)[100]; // new Object[100];

                if( TFR_check(4, 1 == s.allocs) &&
                    TFR_check(4, 100 == counters.objects) ) {

                    pml_delete(&s)[o]; // delete[] o;

                    if( TFR_check(4, 0 == s.allocs) &&
                        TFR_check(4, 0 == counters.objects) ) {
                        return TFR_true;
                    }
                }
            }
        }
    }

    return TFR_false;
}


//------------------------------------------------------------------------------

struct SetAllocatorTester {

    // Passing in alloc so we can check that it's the same as m_alloc. Normally,
    // you (probably) wouldn't pass alloc to the ctor when using a mechanism like
    // PML_REGISTER_SET_ALLOCATOR().
    SetAllocatorTester(::pml::Allocator *alloc):
        m_data(0),
        m_passed(false) {

        if(m_alloc) {
            // we don't need to rely on the ctor passing the alloc in: m_alloc
            // is usable already.
            m_data = pml_new<int>(m_alloc)[36]; // new int[36];
        }

        m_passed = (alloc == m_alloc);
    }

    ~SetAllocatorTester() {
        pml_delete(m_alloc)[m_data]; // delete[] m_data;
    }

    void destroy() {
        pml_delete(m_alloc)(this); // delete this;
    }

    bool passed() const { return m_passed; }
    void *data() const { return m_data; }

    void set_allocator(::pml::Allocator *alloc) { m_alloc = alloc; }
    PML_REGISTER_SET_ALLOCATOR(set_allocator);

private:
    ::pml::Allocator *m_alloc;
    int *m_data;
    bool m_passed;
};


TFR_Bool test_set_allocator() {

    TFR_Bool result = TFR_false;
    MyAllocator s;

    if(TFR_check(4, 0 == s.allocs)) {

        // normally, you wouldn't pass the allocator into the ctor, but for
        // testing, it allows us to check the allocator was set correctly.
        SetAllocatorTester *sat = pml_new<SetAllocatorTester>(&s)(&s); // new SetAllocatorTester(&s)

        if(sat && TFR_check(4, 2 == s.allocs)) {
            result =
                TFR_check(3, sat->passed()) &&
                TFR_check(3, sat->data());

            sat->destroy();

            result &= TFR_check(4, 0 == s.allocs);
        }
    }

    return result;
}


//------------------------------------------------------------------------------

void declare_pml_tests() {

    TFR_SUITE_DECLARE_M("pml::c++", pml_open, pml_close);
    TFR_SUITE_ADD_M(test_malloc);
    TFR_SUITE_ADD_M(test_new);
#ifdef PML_EMPTY_NEW_S
    TFR_SUITE_ADD_M(test_empty_new);
#endif//PML_EMPTY_NEW_S
#ifdef PML_HAS_CPP11
    TFR_SUITE_ADD_M(test_new_8args);
#endif//PML_HAS_CPP11
    TFR_SUITE_ADD_M(test_newa);
    TFR_SUITE_ADD_M(test_new_array);
    TFR_SUITE_ADD_M(test_iallocator);
    TFR_SUITE_ADD_M(test_iallocator2);
    TFR_SUITE_ADD_M(test_set_allocator);
}


} // namespace pml
} // namespace tests
