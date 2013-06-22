#ifndef PML_MALLOC_H
#define PML_MALLOC_H

/** \file pml/malloc.h
 *  The main header for the Proxy Memory Layer (PML) API.
 *
 *  Provides a set of C and C++ memory routines with equivalent functionality to
 *  malloc()/free(), calloc(), realloc(), new/delete and new[]/delete[]. Any
 *  allocations/frees can be routed to user routines using the hook routines,
 *  there is also the possibility to install a debug hook as well for tracing/
 *  monitoring puposes.
 */

#include "pml/malloc_impl.h" /* implementation details */

/*----------------------------------------------------------------------------*/
/* SHARED API BEGINS */

PML_BEGIN_NAMESPACE

/** Allocation hint.
 */
typedef const char *PML_TYPE(Hint);


/** Forward declaration of types.
 */
PML_FORWARD_STRUCT(Allocator);
PML_FORWARD_STRUCT(DebugHookInfo);
PML_FORWARD_STRUCT(AssertHookInfo);


/** Debug hook type.
 *  If a debug hook is installed, the first parameter passed to it describes
 *  the type of memory event taking place.
 */
PML_ENUM(DebugHookType,

    PML_VALUE(MALLOC)
    PML_VALUE(FREE)
    PML_VALUE(CALLOC)
    PML_VALUE(REALLOC)

    PML_VALUE(NEW)
    PML_VALUE(DELETE)
    PML_VALUE(NEWA)
    PML_VALUE(DELETEA)
);


/*----------------------------------------------------------------------------*/
/* DebugHookInfo */

PML_STRUCT(
    DebugHookInfo,

    PML_TYPE(DebugHookType) type; /**< Hook type. */
    size_t count; /** Number of objects (de)allocated. */
    size_t size; /** Size of allocation */
    void *ptr; /**  */
    void *in; /** Pointer to memory (free/realloc) */
    PML_TYPE(Allocator) *alloc; /** Allocator. */
    PML_TYPE(Hint) hint; /** Hint. */
);


/*----------------------------------------------------------------------------*/
/* AssertHookInfo */

PML_STRUCT(
    AssertHookInfo,

    const char *expr; /**< Expression of failed assertion. */
    const char *file; /**< File holding the assertion. */
    size_t line; /**< Line number. */
);


/*----------------------------------------------------------------------------*/
/* Hooks */

typedef void *(*PML_TYPE(MallocHook))(size_t s,
    PML_TYPE(Allocator) *a, PML_TYPE(Hint) h);
typedef void (*PML_TYPE(FreeHook))(void *p,
    PML_TYPE(Allocator) *a, PML_TYPE(Hint) h);
typedef void *(*PML_TYPE(CallocHook))(size_t c, size_t s,
    PML_TYPE(Allocator) *a, PML_TYPE(Hint) h);
typedef void *(*PML_TYPE(ReallocHook))(void *p, size_t s,
    PML_TYPE(Allocator) *a, PML_TYPE(Hint) h);

typedef void (*PML_TYPE(DebugHook))(const PML_TYPE(DebugHookInfo) *i);
typedef void (*PML_TYPE(AssertHook))(const PML_TYPE(AssertHookInfo) *i);


/*----------------------------------------------------------------------------*/
/* Allocator */

PML_STRUCT(
    Allocator,

    PML_TYPE(MallocHook) malloc;
    PML_TYPE(FreeHook) free;

    PML_TYPE(CallocHook) calloc;
    PML_TYPE(ReallocHook) realloc;
);


PML_END_NAMESPACE

/*----------------------------------------------------------------------------*/
/* Declaration of the API. */

/* As any PML types defined are the same in both languages, we can use the C++
 * types here, and we will link against the C functions just fine.
 * PML_Q_TYPE(Name) refers to the fully namespace-qualified typename (pml::Name)
 */

/*----------------------------------------------------------------------------*/
/* Hooks */

PML_API(bool, set_debug_hook)(PML_Q_TYPE(DebugHook) hook);
PML_API(bool, set_assert_hook)(PML_Q_TYPE(AssertHook) hook);
PML_API(bool, set_malloc_hook)(PML_Q_TYPE(MallocHook) hook);
PML_API(bool, set_free_hook)(PML_Q_TYPE(FreeHook) hook);
PML_API(bool, set_calloc_hook)(PML_Q_TYPE(CallocHook) hook);
PML_API(bool, set_realloc_hook)(PML_Q_TYPE(ReallocHook) hook);


/*----------------------------------------------------------------------------*/
/* C memory API */

PML_API(void*, malloc)(size_t size,
    PML_Q_TYPE(Allocator) *alloc PML_DEFAULT(0), PML_Q_TYPE(Hint) hint PML_DEFAULT(0));
PML_API(void, free)(void *ptr,
    PML_Q_TYPE(Allocator) *alloc PML_DEFAULT(0), PML_Q_TYPE(Hint) hint PML_DEFAULT(0));
PML_API(void*, calloc)(size_t count, size_t size,
    PML_Q_TYPE(Allocator) *alloc PML_DEFAULT(0), PML_Q_TYPE(Hint) hint PML_DEFAULT(0));
PML_API(void*, realloc)(void *ptr, size_t size,
    PML_Q_TYPE(Allocator) *alloc PML_DEFAULT(0), PML_Q_TYPE(Hint) hint PML_DEFAULT(0));


/*----------------------------------------------------------------------------*/
/* Proxy routines for allocators which want to provide these C APIs but only want
 * to override malloc()/free()...
 */

PML_API(void*, emulate_calloc)(size_t count, size_t size,
    PML_Q_TYPE(Allocator) *alloc, PML_Q_TYPE(Hint) hint);
PML_API(void*, emulate_realloc)(void *ptr, size_t size,
    PML_Q_TYPE(Allocator) *alloc, PML_Q_TYPE(Hint) hint);


/*----------------------------------------------------------------------------*/
/* Debugging */

#ifdef PML_DEBUG_HOOK_S
PML_API(void, debug_hook)(PML_Q_TYPE(DebugHookType) type,
    size_t count, size_t size, void *ptr, void *in,
    PML_Q_TYPE(Allocator) *alloc, PML_Q_TYPE(Hint) hint);
#endif/*PML_DEBUG_HOOK_S*/


#ifdef PML_ASSERT_HOOK_S
PML_API(void, assert_hook)(bool test, const char *expr,
    const char *file, size_t line);
#endif/*PML_ASSERT_HOOK_S*/


/*----------------------------------------------------------------------------*/
/* Initialize an Allocator */

PML_INLINE_API(void, init_allocator)(
    PML_Q_TYPE(Allocator) *alloc,
    PML_Q_TYPE(MallocHook) mhk,
    PML_Q_TYPE(FreeHook) fhk,
    PML_Q_TYPE(CallocHook) chk PML_DEFAULT(0),
    PML_Q_TYPE(ReallocHook) rhk PML_DEFAULT(0)) {

    PML_ASSERT(alloc);
    alloc->malloc = mhk;
    alloc->free = fhk;
    alloc->calloc = chk;
    alloc->realloc = rhk;
}


/* SHARED API ENDS */
/*----------------------------------------------------------------------------*/


#ifdef __cplusplus

/*----------------------------------------------------------------------------*/
/* Additional C++-only functionality */

/*----------------------------------------------------------------------------*/
/* PML_REGISTER_SET_ALLOCATOR() API */

PML_BEGIN_NAMESPACE
template<typename T>
struct HasType { typedef void Type; };


template<typename T, typename = void>
struct SetAllocatorTagCheck {

    static void call_set_allocator(T &t, Allocator *a) {}
};


template<typename T>
struct SetAllocatorTagCheck<T,
    typename HasType<typename T::SetAllocatorTag>::Type> {

    static void call_set_allocator(T &t, Allocator *a) {
        t.call_set_allocator_INTERNAL_(a);
    }
};
PML_END_NAMESPACE


/** This macro can be used to specify a public method which can be called on an
 *  object to inform it of the allocator which was used to allocate it.
 *  \note The method will be called _before_ the constructor, so it should not
 *    do anything which relies on the object being constructed.
 *
 *  Macro itself should also go in the public section of the class definition.
 */
#define PML_REGISTER_SET_ALLOCATOR(NAME_) \
    void call_set_allocator_INTERNAL_(::pml::Allocator *a) { NAME_(a); } \
    typedef int SetAllocatorTag


/*----------------------------------------------------------------------------*/
/* Replacement for operator new (0-5 arguments currently supported in C++98).
 *
 * Single allocation:
 *     pml_new<T>(...)() --> new T / new T() 
 *     pml_new<T>(...)(a) --> new T(a) 
 *     pml_new<T>(...)(a, b) --> new T(a, b) 
 *     ...
 *     pml_new<T>(...)(a, b, c, d, e) --> new T(a, b, c, d, e)
 *
 * Array allocation:
 *     pml_new<T>(...)[n] --> new T[n] 
 */


/* This is the function body of the NewResult::operator(), which is expanded in
 * place in the template functions, to avoid having to repeat the code. (This
 * does make it harder to debug though, so try to keep it simple...)
 */
#define PML_NEW_OPERATOR_IMPL(ARGS_) \
     \
    size_t size = sizeof(T); \
    void *ptr = PML_NEW_OPERATOR_ALLOC(); \
     \
    if(ptr) { \
        SetAllocatorTagCheck<T>::call_set_allocator(*(T*)ptr, alloc); \
        new(::PML_Q_TYPE(Placement)(ptr)) T ARGS_; \
    } \
    PML_DEBUG_HOOK(NEW, 1, size, ptr, 0, alloc, hint); \
     \
    return static_cast<T*>(ptr)


#ifdef PML_CHECK_S
/* When check is enabled, we allocate an 'array' of 1 object */
#define PML_NEW_OPERATOR_ALLOC() alloc_n(1, true)
#else/*PML_CHECK_S*/
/* No checking, just allocate space for one T. */
#define PML_NEW_OPERATOR_ALLOC() PML_CALL(malloc)(size, alloc, hint)
#endif/*PML_CHECK_S*/


PML_BEGIN_NAMESPACE
template<typename T>
struct NewResult {

    NewResult(PML_Q_TYPE(Allocator) *a, PML_Q_TYPE(Hint) h):
        alloc(a), hint(h) {}

#ifdef PML_HAS_CPP11

    /* variadic forwarding template operator() */
    template<typename... ARGS>
    T *operator()(ARGS &&...args) {
        PML_NEW_OPERATOR_IMPL((std::forward<ARGS>(args)...));
    }

#else/*PML_HAS_CPP11*/

    /* old-style nonvaridiac template operator()s (0-5 args supported) */
    T *operator()() {

        PML_NEW_OPERATOR_IMPL(());
    }

    template<typename P0>
    T *operator()(PML_NEW_PARAM(P0) p0) {

        PML_NEW_OPERATOR_IMPL((p0));
    }

    template<typename P0, typename P1>
    T *operator()(PML_NEW_PARAM(P0) p0, PML_NEW_PARAM(P1) p1) {

        PML_NEW_OPERATOR_IMPL((p0, p1));
    }

    template<typename P0, typename P1, typename P2>
    T *operator()(PML_NEW_PARAM(P0) p0, PML_NEW_PARAM(P1) p1, PML_NEW_PARAM(P2) p2) {

        PML_NEW_OPERATOR_IMPL((p0, p1, p2));
    }

    template<typename P0, typename P1, typename P2, typename P3>
    T *operator()(PML_NEW_PARAM(P0) p0, PML_NEW_PARAM(P1) p1,
        PML_NEW_PARAM(P2) p2, PML_NEW_PARAM(P3) p3) {

        PML_NEW_OPERATOR_IMPL((p0, p1, p2, p3));
    }

    template<typename P0, typename P1, typename P2, typename P3, typename P4>
    T *operator()(PML_NEW_PARAM(P0) p0, PML_NEW_PARAM(P1) p1,
        PML_NEW_PARAM(P2) p2, PML_NEW_PARAM(P3) p3, PML_NEW_PARAM(P4) p4) {

        PML_NEW_OPERATOR_IMPL((p0, p1, p2, p3, p4));
    }

#endif/*PML_HAS_CPP11*/

    /* TODO: this implementation can't handle types needing alignment > sizeof(size_t) */
    T *operator[](size_t count) {

        /* allocate array plus one 'size_t' to store the array count... */
        void *ptr = alloc_n(count);

        /* call ctors for each object in the array */
        T *arr = static_cast<T*>(ptr);
        for(size_t i = 0; i < count; i++) {
            new(PML_Q_TYPE(Placement)(arr + i)) T;
        }

        size_t size = (sizeof(T) * count) + sizeof(count);
        PML_DEBUG_HOOK(NEWA, count, size, ptr, 0, alloc, hint);

        return static_cast<T*>(ptr);
    }

private:
    PML_Q_TYPE(Allocator) *alloc;
    PML_Q_TYPE(Hint) hint;

    void *alloc_n(size_t count, bool checked_single = false) {

#ifdef PML_CHECK_S
        if(checked_single) { PML_ASSERT(1 == count); }
#endif/*PML_CHECK_S*/

        /* allocate array plus one 'size_t' to store the array count... */
        size_t size = (sizeof(T) * count) + sizeof(count);
        void *ptr = PML_CALL(malloc)(size, alloc, hint);
        size_t *data = static_cast<size_t*>(ptr);

#ifdef PML_CHECK_S
        /* Write a 'count' of -1 for a single alloc if checking is enabled */
        if(checked_single) {
            count = static_cast<size_t>(-1);
        }
#endif/*PML_CHECK_S*/

        *data = count;
        ptr = data + 1;

        return ptr;
    }
};
PML_END_NAMESPACE


template<typename T>
inline PML_Q_TYPE(NewResult)<T>
pml_new(PML_Q_TYPE(Allocator) *alloc = 0, PML_Q_TYPE(Hint) hint = 0) {

    return PML_Q_TYPE(NewResult)<T>(alloc, hint);
}


template<typename T>
inline PML_Q_TYPE(NewResult)<T> pml_new(PML_Q_TYPE(Hint) hint) {

    return PML_Q_TYPE(NewResult)<T>(0, hint);
}


/*----------------------------------------------------------------------------*/
/** Delete replacement:
 *
 *  Single:
 *      pml_delete(...)(p) -> delete p;
 *
 *  Array:
 *      pml_delete(...)[p] -> delete[] p;
 */

PML_BEGIN_NAMESPACE
struct DeleteResult {

    DeleteResult(PML_Q_TYPE(Allocator) *a, PML_Q_TYPE(Hint) h):
        alloc(a), hint(h) {}

    template<typename T>
    void operator()(const T *p) {
        /* `delete` can be used on const pointers, but `free()` takes only
         * `void*`. For now, we'll just use `const_cast` to allow for this.
         */
        if(T *ptr = const_cast<T*>(p)) {
            PML_DEBUG_HOOK(DELETE, 1, 0, ptr, 0, alloc, hint);
#ifdef PML_CHECK_S
            size_t count;
            void *data = delete_n(ptr, count, false);
            PML_CALL(free)(data, alloc, hint);
#else/*PML_CHECK_S*/
            ptr->~T();
            PML_CALL(free)(ptr, alloc, hint);
#endif/*PML_CHECK_S*/
        }
    }

    template<typename T>
    void operator[](const T *p) {
        /* `delete` can be used on const pointers, but `free()` takes only
         * `void*`. For now, we'll just use `const_cast` to allow for this.
         */
        if(T *ptr = const_cast<T*>(p)) {
            size_t count;
            void *data = delete_n(ptr, count);
            PML_DEBUG_HOOK(DELETEA, count, 0, ptr, 0, alloc, hint);
            PML_CALL(free)(data, alloc, hint);
        }
    }

private:
    PML_Q_TYPE(Allocator) *alloc;
    PML_Q_TYPE(Hint) hint;

    template<typename T>
    void *delete_n(T *ptr, size_t &count, bool array = true) {
        /* retrieve array count from just in front of the passed-in pointer... */
        void *p = ptr;
        size_t *data = static_cast<size_t*>(p) - 1;
        count = *data;

#ifdef PML_CHECK_S
        if(static_cast<size_t>(-1) == count) {
            PML_ASSERT(!array);
            count = 1;
        } else {
            PML_ASSERT(array);
        }
#endif/*PML_CHECK_S*/

        /* call dtors in reverse order than ctors were called... */
        for(size_t i = 0, j = count - 1; i < count; i++, j--) {
            ptr[j].~T();
        }

        return data;
    }

};
PML_END_NAMESPACE


inline PML_Q_TYPE(DeleteResult) pml_delete(
    PML_Q_TYPE(Allocator) *alloc = 0, PML_Q_TYPE(Hint) hint = 0) {

    return PML_Q_TYPE(DeleteResult)(alloc, hint);
}


inline PML_Q_TYPE(DeleteResult) pml_delete(PML_Q_TYPE(Hint) hint) {
    return PML_Q_TYPE(DeleteResult)(0, hint);
}


/*----------------------------------------------------------------------------*/
/* IAllocator */

PML_BEGIN_NAMESPACE
/** An interface wrapper around Allocator which allows more typical C++ idioms
 *  to be used - virtual functions, inheritance, etc...
 */
struct IAllocator: Allocator {

    IAllocator() {
        PML_CALL(init_allocator)(this,
            static_malloc, static_free, static_calloc, static_realloc);
    };

    virtual ~IAllocator() {}

    /* Need to provide malloc()/free() */
    virtual void *malloc(size_t size, Hint h = 0) = 0;
    virtual void free(void *ptr, Hint h = 0) = 0;

    /* calloc()/realloc() overrides are optional */
    virtual void *calloc(size_t count, size_t size, Hint h = 0) {
        return PML_CALL(emulate_calloc)(count, size, this, h);
    }

    virtual void *realloc(void *ptr, size_t size, Hint h = 0) {
        return PML_CALL(emulate_realloc)(ptr, size, this, h);
    }

private:
    static inline void *static_malloc(size_t size, Allocator *a, Hint h) {
        return static_cast<IAllocator*>(a)->malloc(size, h);
    }

    static inline void static_free(void *ptr, Allocator *a, Hint h) {
        return static_cast<IAllocator*>(a)->free(ptr, h);
    }

    static inline void *static_calloc(size_t count, size_t size, Allocator *a, Hint h) {
        return static_cast<IAllocator*>(a)->calloc(count, size, h);
    }

    static inline void *static_realloc(void *ptr, size_t size, Allocator *a, Hint h) {
        return static_cast<IAllocator*>(a)->realloc(ptr, size, h);
    }
};
PML_END_NAMESPACE


/*----------------------------------------------------------------------------*/
/* "Hint-only" API... */

inline void *PML_APINAME(malloc)(size_t size, PML_Q_TYPE(Hint) hint) {
    return PML_CALL(malloc)(size, 0, hint);
}


inline void PML_APINAME(free)(void *ptr, PML_Q_TYPE(Hint) hint) {
    PML_CALL(free)(ptr, 0, hint);
}


inline void *PML_APINAME(calloc)(size_t count, size_t size, PML_Q_TYPE(Hint) hint) {

    return PML_CALL(calloc)(count, size, 0, hint);
}


inline void *PML_APINAME(realloc)(void *ptr, size_t size, PML_Q_TYPE(Hint) hint) {
    return PML_CALL(realloc)(ptr, size, 0, hint);
}


#else/*__cplusplus*/

/*----------------------------------------------------------------------------*/
/* Additional C-only functionality */

#ifdef PML_HAS_C99
/** Variadic macro versions of pml_malloc(), pml_free(), pml_calloc(),
 *  pml_realloc() and pml_init_allocator() which can be used to simulate
 *  optional/default arguments for these functions.
 *  (On a C11 compiler, we do additional testing with _Generic() to allow the
 *  user to specify just a hint parameter.)
 */
#define pml_malloc(...) \
    pml_VA_EXPAND(pml_malloc, __VA_ARGS__)

#define pml_free(...) \
    pml_VA_EXPAND(pml_free, __VA_ARGS__)

#define pml_calloc(...) \
    pml_VA_EXPAND(pml_calloc, __VA_ARGS__)

#define pml_realloc(...) \
    pml_VA_EXPAND(pml_realloc, __VA_ARGS__)

#define pml_init_allocator(...) \
    pml_VA_EXPAND(pml_init_allocator, __VA_ARGS__)


/*----------------------------------------------------------------------------*/
/* Backend macros for the above */

#define pml_malloc_1(SIZE_) pml_malloc(SIZE_, 0, 0)
#define pml_malloc_2(SIZE_, ARG_) pml_malloc_2args(SIZE_, ARG_)
#define pml_malloc_3(SIZE_, ALLOC_, HINT_) pml_malloc(SIZE_, ALLOC_, HINT_)

#define pml_free_1(PTR_) pml_free(PTR_, 0, 0)
#define pml_free_2(PTR_, ARG_) pml_free_2args(PTR_, ARG_)
#define pml_free_3(PTR_, ALLOC_, HINT_) pml_free(PTR_, ALLOC_, HINT_)

#define pml_calloc_2(COUNT_, SIZE_) pml_calloc(COUNT_, SIZE_, 0, 0)
#define pml_calloc_3(COUNT_, SIZE_, ARG_) pml_calloc_3args(COUNT_, SIZE_, ARG_)
#define pml_calloc_4(COUNT_, SIZE_, ALLOC_, HINT_) \
    pml_calloc(COUNT_, SIZE_, ALLOC_, HINT_)

#define pml_realloc_2(PTR_, SIZE_) pml_realloc(PTR_, SIZE_, 0, 0)
#define pml_realloc_3(PTR_, SIZE_, ARG_) pml_realloc_3args(PTR_, SIZE_, ARG_)
#define pml_realloc_4(PTR_, SIZE_, ALLOC_, HINT_) \
    pml_realloc(PTR_, SIZE_, ALLOC_, HINT_)

#define pml_init_allocator_3(ALLOC_, MALLOC_, FREE_) \
    pml_init_allocator(ALLOC_, MALLOC_, FREE_, 0, 0)
#define pml_init_allocator_4(ALLOC_, MALLOC_, FREE_, CALLOC_) \
    pml_init_allocator(ALLOC_, MALLOC_, FREE_, CALLOC_, 0)
#define pml_init_allocator_5(ALLOC_, MALLOC_, FREE_, CALLOC_, REALLOC_) \
    pml_init_allocator(ALLOC_, MALLOC_, FREE_, CALLOC_, REALLOC_)


#ifdef PML_HAS_C11

/* If we have C11 capablities, use _Generic() to provide "overloads" of the 2 or
 * 3 argument macros to allow (e.g.) pml_malloc(size, "hint") et al.
 */

#define pml_malloc_2args(SIZE_, ARG_) _Generic((ARG_), \
    PmlAllocator*: pml_malloc(SIZE_, ARG_, 0), \
    default: pml_malloc(SIZE_, 0, ARG_))

#define pml_free_2args(PTR_, ARG_) _Generic((ARG_), \
    PmlAllocator*: pml_free(PTR_, ARG_, 0), \
    default: pml_free(PTR_, 0, ARG_))

#define pml_calloc_3args(COUNT_, SIZE_, ARG_) _Generic((ARG_), \
    PmlAllocator*: pml_calloc(COUNT_, SIZE_, ARG_, 0), \
    default: pml_calloc(COUNT_, SIZE_, 0, ARG_))

#define pml_realloc_3args(PTR_, SIZE_, ARG_) _Generic((ARG_), \
    PmlAllocator*: pml_realloc(PTR_, SIZE_, ARG_, 0), \
    default: pml_realloc(PTR_, SIZE_, 0, ARG_))

#else/*PML_HAS_C11*/

/* If we don't have C11, then you'll just have to specify a null ALLOC_ before
 * any hint parameter...
 */
#define pml_malloc_2args(SIZE_, ALLOC_) pml_malloc(SIZE_, ALLOC_, 0)
#define pml_free_2args(PTR_, ALLOC_) pml_free(PTR_, ALLOC_, 0)
#define pml_calloc_3args(COUNT_, SIZE_, ALLOC_) pml_calloc(COUNT_, SIZE_, ALLOC_, 0)
#define pml_realloc_3args(PTR_, SIZE_, ALLOC_) pml_realloc(PTR_, SIZE_, ALLOC_, 0)

#endif/*PML_HAS_C11*/
#endif/*PML_HAS_C99*/

#endif/*__cplusplus*/

#endif/*PML_MALLOC_H*/
