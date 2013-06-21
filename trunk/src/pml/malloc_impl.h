#ifndef PML_MALLOC_IMPL_H
#define PML_MALLOC_IMPL_H

/** \file pml/malloc_impl.h
 *  Implementation details for "pml/malloc.h".
 *
 *  We test for the version(s) of C and C++ supported by the compiler, and define
 *  compatibility layer, which allows us to specify a few types and functions
 *  which are sharable/compatible between the two languages.
 *  We also provide additional infrastructure for some language-specific features
 *  of the API.
 */

/*----------------------------------------------------------------------------*/
/* Versioning */

/** Testing for C++11, C99 and C11. Some compilers aren't reporting these yet,
 *  so it's possible to #define these flags externally if you know your compiler
 *  supports the features we need.
 */
#if __cplusplus >= 201103L
#ifndef PML_HAS_CPP11
#define PML_HAS_CPP11 /* C++11 */
#endif/*PML_HAS_CPP11*/
#endif/*__cplusplus >= 201103L*/

#if __STDC_VERSION__ >= 199901L
#ifndef PML_HAS_C99
#define PML_HAS_C99 /* C99 */
#endif/*PML_HAS_C99*/
#endif/*__STDC_VERSION__ >= 199901L*/

#if __STDC_VERSION__ >= 201112L
#ifndef PML_HAS_C11
#define PML_HAS_C11 /* C11 */
#endif/*PML_HAS_C11*/
#endif/*__STDC_VERSION__ >= 199901L*/


/*----------------------------------------------------------------------------*/
/* Headers */

#include <stddef.h> /* for size_t */
#include <stdbool.h> /* for bool/true/false (C99) */
#include <assert.h>
#ifdef PML_HAS_CPP11
#include <utility> /* for std::forward() */
#endif/*PML_HAS_CPP11*/

#ifndef PML_MALLOC_H
#error "Please use \"pml/malloc.h\", don't include this directly"
#endif/*PML_MALLOC_H*/


/*----------------------------------------------------------------------------*/
/* Settings */

/* Enable debug hook (unless PML_NO_DEBUG_HOOK_S is defined) */
#ifndef PML_DEBUG_HOOK_S
#ifndef PML_NO_DEBUG_HOOK_S
#define PML_DEBUG_HOOK_S
#endif/*PML_NO_DEBUG_HOOK_S*/
#endif/*PML_DEBUG_HOOK_S*/

/* Enable assert hook (unless PML_NO_ASSERT_HOOK_S is defined) */
#ifndef PML_ASSERT_HOOK_S
#ifndef PML_NO_ASSERT_HOOK_S
#define PML_ASSERT_HOOK_S
#endif/*PML_NO_ASSERT_HOOK_S*/
#endif/*PML_ASSERT_HOOK_S*/

/* Enable hint expansions (unless PML_NO_HINT_S is defined) */
#ifndef PML_HINT_S
#ifndef PML_NO_HINT_S
#define PML_HINT_S
#endif/*PML_NO_HINT_S*/
#endif/*PML_HINT_S*/

/* Enable checks (unless PML_NO_CHECK_S is defined) */
#ifndef PML_CHECK_S
#ifndef PML_NO_CHECK_S
#define PML_CHECK_S
#endif/*PML_NO_CHECK_S*/
#endif/*PML_CHECK_S*/


/*----------------------------------------------------------------------------*/
/* API namespace - contains every type and enum value in C++
 * (e.g. pml::DebugHook, pml::MALLOC).
 * Also prepended to enum values in C (e.g. pml_MALLOC).
 * Prepended to functions in both C/C++ (e.g. pml_malloc()).
 */
#ifndef PML_API_NAMESPACE
#define PML_API_NAMESPACE pml
#endif/*PML_API_NAMESPACE*/

/* Type namespace - prepended to types in C (e.g. PmlDebugHook).
 */
#ifndef PML_TYPE_NAMESPACE
#define PML_TYPE_NAMESPACE Pml
#endif/*PML_TYPE_NAMESPACE*/


/* Option to remove the debug hook from release code */
#ifdef PML_DEBUG_HOOK_S
#define PML_DEBUG_HOOK(TYPE_, COUNT_, SIZE_, PTR_, IN_, ALLOC_, HINT_) \
    (PML_CALL(debug_hook)(PML_Q_NAME(TYPE_), COUNT_, SIZE_, PTR_, IN_, ALLOC_, HINT_))

#else/*PML_DEBUG_HOOK_S*/
#define PML_DEBUG_HOOK(TYPE_, COUNT_, SIZE_, PTR_, IN_, ALLOC_, HINT_)
#endif/*PML_DEBUG_HOOK_S*/

/* Assert macro */
#ifdef PML_ASSERT_HOOK_S
#define PML_ASSERT(EXPR_) \
    (PML_CALL(assert_hook)(EXPR_, #EXPR_, __FILE__, __LINE__))

#else/*PML_ASSERT_HOOK_S*/
/* default to assert (user can still override by #defining it) */
#ifndef PML_ASSERT
#define PML_ASSERT assert
#endif/*PML_ASSERT*/
#endif/*PML_ASSERT_HOOK_S*/


#ifdef PML_HINT_S
#define PML_HINT(HINT_) pml_MAKE_HINT_0(HINT_, __FILE__, __LINE__)
#else/*PML_HINT_S*/
#define PML_HINT(HINT_) 0
#endif/*PML_HINT_S*/



/*----------------------------------------------------------------------------*/
/* Cross-language compatibility */

/** API (function) names. These wil have the PML_API_NAMESPACE prepended in both
 *  languages.
 */
#define PML_APINAME(NAME_) pml_APINAME_0(PML_API_NAMESPACE, NAME_)

/** API declaration. */
#define PML_API(RETURN_, NAME_) PML_EXTERN RETURN_ PML_APINAME(NAME_)

/** Type names in C. */
#define PML_C_TYPE(NAME_) pml_C_TYPE_0(PML_TYPE_NAMESPACE, NAME_)

/** Language agnostic API definition (inline). */
#define PML_INLINE_API(RETURN_, NAME_) PML_INLINE RETURN_ PML_APINAME(NAME_)

/* Language agnostic enum definition. */
#define PML_ENUM(NAME_, VALUES) \
    typedef enum PML_TYPE(NAME_) { VALUES } PML_TYPE(NAME_)

/* Language agnostic enum value (for use inside PML_ENUM()). Note that this macro
 * appends a comma to the enum value definition, so we don't need PML_ENUM() to
 * be variadic (C++98 support).
 */
#define PML_VALUE(NAME_) PML_NAME(NAME_),


#ifdef __cplusplus

/* C++ specifics */

#define PML_DEFAULT(V) = V
#define PML_EXTERN extern "C"
#define PML_INLINE inline
#define PML_BEGIN_NAMESPACE namespace PML_API_NAMESPACE {
#define PML_END_NAMESPACE }

#define PML_CALL(API_) ::PML_APINAME(API_)

#define PML_TYPE(NAME_) NAME_
#define PML_NAME(NAME_) NAME_
#define PML_Q_TYPE(NAME_) PML_API_NAMESPACE::NAME_
#define PML_Q_NAME(NAME_) PML_API_NAMESPACE::NAME_

#define PML_FORWARD_STRUCT(NAME_) struct PML_TYPE(NAME_)
#define PML_STRUCT(NAME_, DATA_) \
    struct PML_TYPE(NAME_) { DATA_ }

/* Pass template args to (C++98) 'new' operators by const ref unless the switch
 * PML_NEW_BY_VALUE_S is #defined.
 */
#ifdef PML_NEW_BY_VALUE
#define PML_NEW_PARAM(NAME_) NAME_
#else//PML_NEW_BY_VALUE
#define PML_NEW_PARAM(NAME_) const NAME_&
#endif//PML_NEW_BY_VALUE

#else/*__cplusplus*/

/* C specifics */

#define PML_DEFAULT(V)
#define PML_EXTERN extern
#define PML_INLINE static inline
#define PML_BEGIN_NAMESPACE
#define PML_END_NAMESPACE

#define PML_CALL(API_) PML_APINAME(API_)

#define PML_TYPE(NAME_)  PML_C_TYPE(NAME_)
#define PML_NAME(NAME_) PML_APINAME(NAME_)
#define PML_Q_TYPE(NAME_) PML_C_TYPE(NAME_)
#define PML_Q_NAME(NAME_) PML_APINAME(NAME_)

#define PML_FORWARD_STRUCT(NAME_) typedef struct PML_TYPE(NAME_) PML_TYPE(NAME_)
#define PML_STRUCT(NAME_, DATA_) \
    struct PML_TYPE(NAME_) { DATA_ }

#endif/*__cplusplus*/


/*----------------------------------------------------------------------------*/
/* Placement new (C++) */

#ifdef __cplusplus
PML_BEGIN_NAMESPACE
struct Placement {
    explicit Placement(void *p): ptr(p) {}
    void *ptr;
};
PML_END_NAMESPACE


inline void *operator new(size_t size, const PML_Q_TYPE(Placement) &p) {
    return p.ptr;
}
#endif/*__cplusplus*/


#ifdef PML_HAS_C99
/*----------------------------------------------------------------------------*/
/* infrastructure for variadic macros to provide default arguments in C */

/** Expand into a new macro call where the new macro name contains the length of
 *  __VA_ARGS__ (and can be used to provide default args to the original function).
 */
#define pml_VA_EXPAND(NAME_, ...) \
    pml_VA_NAME(NAME_,__VA_ARGS__)(__VA_ARGS__)

/** Expands into [NAME_]_[VA_COUNT], which is a macro which will supply the
 *  default args for the function NAME_.
 */
#define pml_VA_NAME(NAME_, ...) \
    pml_JOIN3(NAME_, _, pml_VA_COUNT(__VA_ARGS__))

/** Join (expanded) A to (expanded) B */
#define pml_JOIN(A, B) pml_JOIN_0(A, B)

/** Join (expanded) A, (expanded) B and (expanded) C */
#define pml_JOIN3(A, B, C) pml_JOIN(A, pml_JOIN(B, C))

/** Count (the number of commas plus one in) __VA_ARGS__.
 *  We support up to 12 args for now, the max we actually need is 5, but there's
 *  some room for future expansion.
 */
#define pml_VA_COUNT(...) pml_VA_COUNT_0(__VA_ARGS__)

#endif/*PML_HAS_C99*/


/*----------------------------------------------------------------------------*/
/* Backend macros follow */

#define pml_APINAME_0(NS_, NAME_) pml_APINAME_1(NS_, NAME_)
#define pml_APINAME_1(NS_, NAME_) NS_##_##NAME_

#define pml_C_TYPE_0(NS_, NAME_) pml_C_TYPE_1(NS_, NAME_)
#define pml_C_TYPE_1(NS_, NAME_) NS_##NAME_

#define pml_JOIN_0(A, B) pml_JOIN_1(A, B)
#define pml_JOIN_1(A, B) A##B

#ifdef PML_HAS_C99
#define pml_VA_COUNT_0(...) pml_VA_COUNT_1(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define pml_VA_COUNT_1(A, B, C, D, E, F, G, H, I, J, K, L, M, ...) M
#endif/*PML_HAS_C99*/

#define pml_MAKE_HINT_0(H_, F_, L_) pml_MAKE_HINT_1(H_, F_, L_)
#define pml_MAKE_HINT_1(H_, F_, L_) (F_ "(" #L_ "): \"" H_ "\"")

#endif/*PML_MALLOC_IMPL_H*/
