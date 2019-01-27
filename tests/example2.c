#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* AB_vector config */
#define AB_VEC_INCLUDE_USERDATA

/* Custom assertion function */
static void my_assert(int cond) {
    if (!cond)
        abort();
}
#define AB_VEC_ASSERT(cond) my_assert(cond)

/* Custom allocation function */
static void *my_realloc(void *ptr, size_t old_size, size_t new_size, void *userdata) {
    void *new_ptr = realloc(ptr, new_size);
    const char *name = userdata;
    fprintf(stderr, "Called with old-size = %lu, new-size = %lu, name = %s\n",
            old_size, new_size, name);

    if (new_size > old_size)
        memset((char *)new_ptr + old_size, 0x0, new_size - old_size);

    return new_ptr;
}
#define AB_VEC_REALLOC(ptr, old_size, new_size, userdata) \
    my_realloc(ptr, old_size, new_size, userdata)

/* Custom free function */
static void my_free(void *ptr, size_t size, void *userdata) {
    const char *name = userdata;
    fprintf(stderr, "Freeing %s - %lu\n", name, size);
    free(ptr);
}
#define AB_VEC_FREE(ptr, size, userdata) my_free(ptr, size, userdata)

/* Custom size-type */
#define AB_VEC_SIZE_T unsigned short

/* Custom roundup function */
static unsigned short my_roundup(unsigned short x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x++;
    return x;
}
#define AB_VEC_SIZE_T_ROUNDUP(x) my_roundup(x)

#include <AB_vector.h>

int main(void)
{
    int i;
    AB_vec(int) vec = AB_VEC_INIT;
    AB_vec(int) vec2 = AB_VEC_INIT;

    AB_vec_userdata(&vec) = "my vector!";

    AB_vec_insert(&vec, 19, 2);

    for (i = 0; i < 20; i++) {
        int *v;
        AB_vec_push(&vec, i);
        v = AB_vec_pushp(&vec);
        my_assert(v != NULL);
        *v = i * 2;
    }

    AB_vec_userdata(&vec2) = "Another vector";
    AB_vec_copy(&vec2, &vec);
    AB_vec_userdata(&vec2) = "Another vector";

    AB_vec_at(&vec, 18) = 3;

    i = AB_vec_size(&vec) - 1;
    while (AB_vec_size(&vec) > 0) {
        printf("pop %d - %d\n", i, AB_vec_pop(&vec));
        i--;
    }

    while (AB_vec_size(&vec2) > 0) {
        printf("pop %d - %d\n", i, AB_vec_pop(&vec2));
        i--;
    }

    AB_vec_destroy(&vec);
    AB_vec_destroy(&vec2);
    return 0;
}
