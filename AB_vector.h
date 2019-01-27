/** @file AB_vector.h
 * @brief Utility header defining a generic resizable vector
 *
 * Many of the things that are to be used in this header are macros that
 * work due to the storage of type information in the vector themselves.
 * This does unfortunately mean that mismatching vector types could cause
 * runtime errors, as there is no way to detect them at compile time.
 *
 * This library is heavily inspired by kvec.h which is a part of klib.
 * This header improves on the initial ideas by removing the need to specify
 * the types in any of the vector functions, and introducing inline
 * functions to hopefully make the source more readable. More compile options
 * are also available for customizable memory management and compatibility.
 *
 * Macro-options:
 *  - AB_VEC_INLINE
 *    Define this macro to be the decorator for inline functions. Tries to
 *    guess whether @c inline or @c __inline__ are available, but try to define
 *    this macro when in ANSI mode.
 *
 *  - AB_VEC_ASSERT(cond)
 *    Define this macro to be the runtime check function. Pulls in
 *    assert.h for @c assert as fallback.
 *
 *  - AB_VEC_USERDATA
 *    Define this macro to insert an additional member into the vector.
 *    This member is passed to the memory allocation functions
 *
 *  - AB_VEC_REALLOC(ptr, old_size, new_size, userdata)
 *    Define this macro to customize the memory usage of the vector. Without
 *    AB_VEC_USERDATA, the userdata argument is always NULL and can be ignored.
 *    Pulls in stdlib.h for @c realloc as fallback, so make sure you define
 *    both AB_VEC_REALLOC and AB_VEC_FREE if you change one.
 *
 *  - AB_VEC_FREE(ptr, size, userdata)
 *    Define this macro to customize memory usage of the vector. Same rules
 *    apply as for AB_VEC_REALLOC. Pulls in stdlib.h for @c free as fallback.
 *
 *  - AB_VEC_SIZE_T
 *    Define this macro to the type to use as the capacity counter. By default
 *    this uses @c size_t. However this may be inefficient as most uses won't
 *    exceed 65,535 which is the max value of an @code unsigned short @endcode.
 *
 *  - AB_VEC_SIZE_T_ROUNDUP(x)
 *    Define this macro as a function from @c AB_VEC_SIZE_T to @c AB_VEC_SIZE_T
 *    which determines the next size of the vector when resizing. This is used
 *    in @c AB_vec_insert() when trying to index past the vector's size. Defaults
 *    to an inefficient version that works on @c size_t. If you use
 *    @c AB_vec_insert(), try to define this more efficiently, as I need to
 *    worry about non-fixed types.
 *
 */
#ifndef AMBER_UTIL_VECTOR_H
#define AMBER_UTIL_VECTOR_H

#include <stddef.h> /* size_t, NULL */
#include <string.h> /* memcpy */

/**************************************************************************
 *
 * User-Defined / Configuration Macros
 *
 *************************************************************************/

#ifndef AB_VEC_ASSERT
# include <assert.h>
/** @brief Runtime assertion macro for the header
 * @note This macro can be overidden
 * This macro is heavily used, so I would suggest disabling it for
 * release builds. Make sure you put something there when disabling, like
 * @c (void)0 to aboid compilation errors. The standard @c assert function
 * does this well by default.
 * @hideinitializer
 */
# define AB_VEC_ASSERT(cond) assert(cond)
#endif /* AB_VEC_ASSERT */

/** @brief Decorator for functions that should be marked inline
 * @note This macro can be overidden
 */
#ifndef AB_VEC_INLINE
# if __STDC_VERSION__ >= 199901L || defined(__DOXYGEN__)
#  define AB_VEC_INLINE inline
# elif __GNUC__
#  define AB_VEC_INLINE __inline__
# else
#  define AB_VEC_INLINE
# endif
#endif /* AB_VEC_INLINE */

#if !defined(AB_VEC_REALLOC) || !defined(AB_VEC_FREE)
# include <stdlib.h>
#endif /* !defined(AB_VEC_REALLOC) || !defined(AB_VEC_FREE) */

/** @brief Memory allocation function for the header
 * @note This macro can be overidden
 */
#ifndef AB_VEC_REALLOC
# ifndef AB_VEC_INCLUDE_USERDATA
#  define AB_VEC_REALLOC(ptr, old_size, new_size) realloc(ptr, new_size)
# else
#  define AB_VEC_REALLOC(ptr, old_size, new_size, userdata) realloc(ptr, new_size)
# endif
#endif /* AB_VEC_REALLOC */

/** @brief Memory de-allocation function for the header
 * @note This macro can be overidden
 */
#ifndef AB_VEC_FREE
# ifndef AB_VEC_INCLUDE_USERDATA
#  define AB_VEC_FREE(ptr, size) free(ptr)
# else
#  define AB_VEC_FREE(ptr, size, userdata) free(ptr)
# endif
#endif /* AB_VEC_FREE */

/** @brief Type of capacity storage
 * @note This macro can be overidden
 */
#ifndef AB_VEC_SIZE_T
# define AB_VEC_SIZE_T size_t
#endif /* ifndef AB_VEC_SIZE_T */

/** @brief Function to determine the next power of 2 given an index
 * @param x The value of type @c AB_VEC_SIZE_T to be rounded up
 * @note This macro can be overidden
 * @hideinitializer
 */
#ifndef AB_VEC_SIZE_T_ROUNDUP
# define AB_VEC_SIZE_T_ROUNDUP(x) AB_vec_roundup_size_t(x)
#endif

/**************************************************************************
 * 
 * Implementation
 *
 *************************************************************************/

/** @cond false */
struct AB_vector_generic {
    AB_VEC_SIZE_T num, capacity;
    void *elems;
#ifdef AB_VEC_INCLUDE_USERDATA
    void *userdata;
#endif
};
/** @endcond */

/** @brief Anonymous structure used for AB_vec functions
 * @param type The element type
 */
#ifdef AB_VEC_INCLUDE_USERDATA
# define AB_vec(type)                                                                              \
    struct { AB_VEC_SIZE_T num, capacity; type *elems; void *userdata; }
#else
# define AB_vec(type)                                                                              \
    struct { AB_VEC_SIZE_T num, capacity; type *elems; }
#endif

/** @brief Initializer list for an AB_vec */
#ifdef AB_VEC_INCLUDE_USERDATA
# define AB_VEC_INIT { 0, 0, NULL, NULL }
#else
# define AB_VEC_INIT { 0, 0, NULL }
#endif

/** @brief Initialize an AB_vec
 * @param vec Pointer to an AB_vec structure
 */
#ifdef AB_VEC_INCLUDE_USERDATA
# define AB_vec_init(vec) do {                                                                     \
    AB_VEC_ASSERT(vec != NULL); \
    (vec)->num = (vec)->capacity = 0; (vec)->elems = NULL; (vec)->userdata = NULL;                 \
} while (0)
#else
# define AB_vec_init(vec) do {                                                                     \
    AB_VEC_ASSERT(vec != NULL); \
    (vec)->num = (vec)->capacity = 0; (vec)->elems = NULL;                                         \
} while (0)
#endif

#if defined(AB_VEC_INCLUDE_USERDATA) || defined(__DOXYGEN__)
/** @brief Access an AB_vec's userdata
 * @param vec Pointer to an AB_vec
 * @return The @c userdata field (as lvalue)
 * @note Only available when @c AB_VEC_INCLUDE_USERDATA is set
 * @hideinitializer
 */
# define AB_vec_userdata(vec) (*(AB_VEC_ASSERT((vec) != NULL), &(vec)->userdata))
#endif

/** @brief Free memory associated with an AB_vec
 * @param vec Pointer to the AB_vec
 * @hideinitializer
 */
#ifdef AB_VEC_INCLUDE_USERDATA
# define AB_vec_destroy(vec)                                                                       \
    (AB_VEC_ASSERT((vec) != NULL), \
     AB_VEC_FREE((vec)->elems, \
         (vec)->capacity * sizeof(*(vec)->elems), (vec)->userdata))
#else
# define AB_vec_destroy(vec)                                                                       \
    (AB_VEC_ASSERT((vec) != NULL), \
     AB_VEC_FREE((vec)->elems, \
         (vec)->capacity * sizeof(*(vec)->elems)))
#endif

/** @brief Access an element at a given index
 * @param vec Pointer to the AB_vec
 * @param idx Index to access
 * @return The element at that index (as an lvalue, ie. can take address)
 * @note @c idx must be a valid index
 */
#define AB_vec_at(vec, idx)                                                                        \
    (*(AB_VEC_ASSERT((vec) != NULL), &(vec)->elems[idx]))

/** @brief Remove the last element of the vector, returning the value
 * @param vec Pointer to the AB_vec
 * @return The removed element
 * @note The vector must be non-empty
 * @hideinitializer
 */
#define AB_vec_pop(vec)                                                                            \
    (AB_VEC_ASSERT((vec)->num > 0), (vec)->elems[--(vec)->num])

/** @brief Query the number of elements in the vector
 * @param vec Pointer to the AB_vec
 * @return The number of elements
 */
#define AB_vec_size(vec)                                                                           \
    (AB_VEC_ASSERT((vec) != NULL), (const AB_VEC_SIZE_T)(vec)->num)

/** @brief Query the current capacity of the vector
 * @param vec Pointer to the AB_vec
 * @return The current number of possible elements storable before @c realloc
 */
#define AB_vec_max(vec)                                                                            \
    (AB_VEC_ASSERT((vec) != NULL), (const AB_VEC_SIZE_T)(vec)->capacity)

static AB_VEC_INLINE int
AB_vec_resize_generic(struct AB_vector_generic *vec, 
        AB_VEC_SIZE_T new_size, AB_VEC_SIZE_T elem_size)
{
    void *new_elems;
    AB_VEC_ASSERT(vec != NULL);
#ifdef AB_VEC_INCLUDE_USERDATA
    new_elems = AB_VEC_REALLOC(vec->elems,
            elem_size * vec->capacity, elem_size * new_size, vec->userdata);
#else
    new_elems = AB_VEC_REALLOC(vec->elems,
            elem_size * vec->capacity, elem_size * new_size);
#endif
    AB_VEC_ASSERT(new_elems != NULL);
    if (new_elems == NULL)
        return 1;

    vec->elems = new_elems;
    vec->capacity = new_size;
    return 0;
}
/** @brief Change the capacity of a vector
 * @param vec Pointer to an AB_vector
 * @param newsize The new vector capacity
 * @return 0 on success, nonzero on error
 * @hideinitializer
 */
#define AB_vec_resize(vec, newsize)                                                                \
    AB_vec_resize_generic((struct AB_vector_generic *)(vec), newsize, sizeof(*(vec)->elems))

static AB_VEC_INLINE int
AB_vec_copy_generic(struct AB_vector_generic *dest,
        const struct AB_vector_generic *src, AB_VEC_SIZE_T entry_size)
{
    AB_VEC_ASSERT(src != NULL);
    AB_VEC_ASSERT(dest != NULL);
    if (dest->capacity < src->capacity) {
        int err = AB_vec_resize_generic(dest, src->capacity, entry_size);
        AB_VEC_ASSERT(!err);
        if (err)
            return 1;
    }
    dest->num = src->num;
    memcpy(dest->elems, src->elems, entry_size * src->capacity);
    return 0;
}
/** @brief Copy a vector from src to dest
 * @param [in, out] dest Pointer to an AB_vec
 * @param [in] src Const pointer to an AB_vec
 * @return 0 on success, nonzero on error
 * @note The vectors must have elements with the same size - it's not checked
 * @note The vector's userdata is NOT copied
 * @hideinitializer
 */
#define AB_vec_copy(dest, src)                                                                     \
    AB_vec_copy_generic((struct AB_vector_generic *)(dest),                                        \
            (const struct AB_vector_generic *)(src), sizeof(*(dest)->elems))

/** @brief Add an element to the end of the vector
 * @param vec Pointer to the AB_vec
 * @param elem The element to insert
 * @return 0 on success, nonzero on error
 * @note Capacity is increased using a left bitshift,
 * so overflow can cause problems.
 * @hideinitializer
 */
#define AB_vec_push(vec, elem)                                                                     \
    (AB_VEC_ASSERT((vec) != NULL), \
     (vec)->num == (vec)->capacity ?                                                               \
         (AB_vec_resize((vec), (vec)->capacity ? (vec)->capacity << 1 : 2) == 0 ?                  \
              ((vec)->elems[(vec)->num++] = (elem), 0)                                             \
              : 1)                                                                                 \
         : ((vec)->elems[(vec)->num++] = (elem), 0))

/** @brief Add an element to the end of the vector, returning a pointer to that spot
 * @param vec Pointer to the AB_vec
 * @return Pointer to the pushed element, or NULL on error
 * @hideinitializer
 */
#define AB_vec_pushp(vec)                                                                          \
    (AB_VEC_ASSERT((vec) != NULL), \
     (vec)->num == (vec)->capacity ?                                                               \
         (AB_vec_resize((vec), (vec)->capacity ? (vec)->capacity << 1 : 2) == 0 ?                  \
              &(vec)->elems[(vec)->num++]                                                          \
              : NULL)                                                                              \
         : &(vec)->elems[(vec)->num++])

/* Default roundup implementation */
static AB_VEC_INLINE AB_VEC_SIZE_T AB_vec_roundup_size_t(AB_VEC_SIZE_T x)
{
    AB_VEC_SIZE_T y = 1;
    if (x == 0)
        return 0;
    while (y <= x)
        y += y;
    return y;
}

static AB_VEC_INLINE int AB_vec_insert_generic(struct AB_vector_generic *vec,
        AB_VEC_SIZE_T idx, AB_VEC_SIZE_T elem_size)
{
    if (vec->capacity <= idx) {
        int err = AB_vec_resize_generic(vec, AB_VEC_SIZE_T_ROUNDUP(idx + 1), elem_size);
        if (err)
            return 1;
    }
    if (vec->num <= idx) {
        vec->num = idx + 1;
    }
    return 0;
}
/** @brief Insert an element at an arbitrary index
 * @param vec Pointer to an AB_vec
 * @param idx index to insert the element, not necessarily an allocated one
 * @param elem the element to insert
 * @return 0 on success, nonzero on error
 * @note If this function expands the vector, skipped indexes are uninitialized
 *  (depending on the @c realloc function)
 * @hideinitializer
 */
#define AB_vec_insert(vec, idx, elem)                                                              \
    (AB_vec_insert_generic((struct AB_vector_generic *)(vec), (idx), sizeof(*(vec)->elems)) == 0 ? \
        (vec)->elems[(idx)] = (elem)                                                               \
        : 1)

#endif /* AMBER_UTIL_VECTOR_H */
