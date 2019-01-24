/** @file AB_vector.h
 * @brief Utility header defining a generic resizable vector
 *
 * Many of the things that are to be used in this header are macros that
 * work due to the storage of type information in the vector themselves.
 * This does unfortunately mean that mismatching vector types could cause
 * runtime errors, as there is no way to detect them at compile time.
 */
#ifndef AMBER_UTIL_VECTOR_H
#define AMBER_UTIL_VECTOR_H

#include <stddef.h> /* size_t */
#include <string.h> /* memcpy */

#ifndef AB_VEC_ASSERT
# include <assert.h> /* assert */
/** @brief Runtime assertion macro for the header
 * @note This macro can be overidden
 * @hideinitializer
 */
# define AB_VEC_ASSERT(cond) assert(cond)
#endif /* AB_VEC_ASSERT */

#ifndef AB_VEC_STATIC_ASSERT
/** @cond false */
# define AB_VEC_STATIC_ASSERT_STRCAT0(a, b, c) a ## b ## c
# define AB_VEC_STATIC_ASSERT_STRCAT(a, b, c) AB_VEC_STATIC_ASSERT_STRCAT0(a, b, c)
/** @endcond */

/** @brief Compiletime assertion macro for the header
 * This macro is called within functions which require specific behavior
 * @note This macro can be overidden
 * @hideinitializer
 */
# define AB_VEC_STATIC_ASSERT(cond, msg) do { \
    char AB_VEC_STATIC_ASSERT_STRCAT(msg, _on_line_, __LINE__)[1 - !(cond) * 2]; \
    (void)(AB_VEC_STATIC_ASSERT_STRCAT(msg, _on_line_, __LINE__)); \
} while (0)
#endif /* AB_VEC_STATIC_ASSERT */

/** @brief Decorator for functions that should be marked inline
 * @note This macro can be overidden
 */
#ifndef AB_VEC_INLINE
# if __STDC_VERSION__ >= 199901L || __DOXYGEN__
#  define AB_VEC_INLINE inline
# elif __GNUC__
#  define AB_VEC_INLINE __inline__
# else
#  define AB_VEC_INLINE
# endif
#endif /* AB_VEC_INLINE */

#ifndef AB_VEC_REALLOC
# include <stdlib.h>
/** @brief Memory allocation function for the header
 * @note This macro can be overidden
 */
# define AB_VEC_REALLOC(ptr, old_size, new_size, userdata) realloc(ptr, new_size)
#endif /* AB_VEC_REALLOC */
#ifndef AB_VEC_FREE
# include <stdlib.h>
/** @brief Memory de-allocation function for the header
 * @note This macro can be overidden
 */
# define AB_VEC_FREE(ptr, size, userdata) free(ptr)
#endif /* AB_VEC_FREE */

/** @cond false */
struct AB_vector_generic {
    size_t num, capacity;
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
# define AB_vec(type)                                                                \
    struct { size_t num, capacity; type *elems; void *userdata; }
#else
# define AB_vec(type)                                                                \
    struct { size_t num, capacity; type *elems; }
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
#define AB_vec_init(vec) do { \
    (vec)->num = (vec)->capacity = 0; (vec)->elems = NULL; \
} while (0)

#ifdef AB_VEC_INCLUDE_USERDATA
/** @brief Access an AB_vec's userdata
 * @param vec Pointer to an AB_vec
 * @return The @c userdata field (as lvalue)
 * @note Only available when @c AB_VEC_INCLUDE_USERDATA is set
 */
# define AB_vec_userdata(vec) ((vec)->userdata)
#endif

/** @brief Free memory associated with an AB_vec
 * @param vec Pointer to the AB_vec
 * @hideinitializer
 */
#ifdef AB_VEC_INCLUDE_USERDATA
# define AB_vec_destroy(vec) \
    AB_VEC_FREE((vec)->elems, (vec)->capacity * sizeof(*(vec)->elems), (vec)->userdata)
#else
# define AB_vec_destroy(vec) \
    AB_VEC_FREE((vec)->elems, (vec)->capacity * sizeof(*(vec)->elems), NULL)
#endif

/** @brief Access an element at a given index
 * @param vec Pointer to the AB_vec
 * @param idx Index to access
 * @return The element at that index (as an lvalue, ie. can take address)
 * @note @c idx must be a valid index
 */
#define AB_vec_at(vec, idx)                                                         \
    ((vec)->elems[idx])

/** @brief Remove the last element of the vector, returning the value
 * @param vec Pointer to the AB_vec
 * @return The removed element
 * @note The vector must be non-empty
 * @hideinitializer
 */
#define AB_vec_pop(vec)                                                             \
    (AB_VEC_ASSERT((vec)->num > 0), (vec)->elems[--(vec)->num])

/** @brief Query the number of elements in the vector
 * @param vec Pointer to the AB_vec
 * @return The number of elements
 */
#define AB_vec_size(vec)                                                            \
    ((const size_t)(vec)->num)

/** @brief Query the current capacity of the vector
 * @param vec Pointer to the AB_vec
 * @return The current number of possible elements storable before @c realloc
 */
#define AB_vec_max(vec)                                                             \
    ((const size_t)(vec)->capacity)

static AB_VEC_INLINE int
AB_vec_resize_generic(struct AB_vector_generic *vec, size_t new_size, size_t elem_size)
{
#ifdef AB_VEC_INCLUDE_USERDATA
    void *new_elems = AB_VEC_REALLOC(vec->elems,
            elem_size * vec->capacity, elem_size * new_size, vec->userdata);
#else
    void *new_elems = AB_VEC_REALLOC(vec->elems,
            elem_size * vec->capacity, elem_size * new_size, NULL);
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
#define AB_vec_resize(vec, newsize) \
    AB_vec_resize_generic((struct AB_vector_generic *)(vec), newsize, sizeof(*(vec)->elems))

static AB_VEC_INLINE int
AB_vec_copy_generic(struct AB_vector_generic *dest,
        struct AB_vector_generic *src, size_t entry_size)
{
    if (dest->capacity < src->capacity) {
        int err = AB_vec_resize_generic(dest, src->capacity, entry_size);
        if (err)
            return 1;
    }
    dest->num = src->num;
    memcpy(dest->elems, src->elems, entry_size * src->capacity);
    return 0;
}
/** @brief Copy a vector from src to dest
 * @param [in, out] dest Pointer to an AB_vec
 * @param [in] src Pointer to an AB_vec
 * @return 0 on success, nonzero on error
 * @note The vectors must have elements with the same size - it's not checked
 * @hideinitializer
 */
#define AB_vec_copy(dest, src) \
    AB_vec_copy_generic((struct AB_vector_generic *)(dest), \
            (struct AB_vector_generic *)(src), sizeof(*(dest)->elems))

/** @brief Add an element to the end of the vector
 * @param vec Pointer to the AB_vec
 * @param elem The element to insert
 * @return 0 on success, nonzero on error
 * @hideinitializer
 */
#define AB_vec_push(vec, elem)                                                    \
    ((vec)->num == (vec)->capacity ?                                              \
         (AB_vec_resize((vec), (vec)->capacity ? (vec)->capacity << 1 : 2) == 0 ? \
              ((vec)->elems[(vec)->num++] = (elem), 0)                            \
              : 1)                                                                \
         : ((vec)->elems[(vec)->num++] = (elem), 0))

/** @brief Add an element to the end of the vector, returning a pointer to that spot
 * @param vec Pointer to the AB_vec
 * @return Pointer to the pushed element, or NULL on error
 * @hideinitializer
 */
#define AB_vec_pushp(vec)                                                         \
    ((vec)->num == (vec)->capacity ?                                              \
         (AB_vec_resize((vec), (vec)->capacity ? (vec)->capacity << 1 : 2) == 0 ? \
              &(vec)->elems[(vec)->num++]                                         \
              : NULL)                                                             \
         : &(vec)->elems[(vec)->num++])

/* Round x up to the nearest power of 2 */
static AB_VEC_INLINE size_t AB_vec_roundup32(size_t x)
{
    AB_VEC_STATIC_ASSERT(sizeof(size_t) >= 8, size_t_too_small);
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    ++x;
    return x;
}

static AB_VEC_INLINE int AB_vec_insert_generic(struct AB_vector_generic *vec, size_t idx, size_t elem_size)
{
    if (vec->capacity <= idx) {
        int err = AB_vec_resize_generic(vec, AB_vec_roundup32(idx + 1), elem_size);
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
#define AB_vec_insert(vec, idx, elem) \
    (AB_vec_insert_generic((struct AB_vector_generic *)(vec), (idx), sizeof(*(vec)->elems)) == 0 ? \
        (vec)->elems[(idx)] = (elem) \
        : 1)

#endif /* AMBER_UTIL_VECTOR_H */
