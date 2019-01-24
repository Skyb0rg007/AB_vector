#include <AB_vector.h>
#include <assert.h>
#include <stdio.h>

int main(void)
{
    AB_vec(int) my_vec = AB_VEC_INIT;
    int i;

    for (i = 0; i < 20; i++) {
        int err = AB_vec_push(&my_vec, i);
        assert(!err);
    }

    for (i = 5; i < 15; i++) {
        int val = AB_vec_at(&my_vec, i);
        printf("%d -> %d\n", i, val);
    }

    while (AB_vec_size(&my_vec) > 0) {
        int val = AB_vec_pop(&my_vec);
        printf("Got value %d\n", val);
    }

    AB_vec_destroy(&my_vec);
    return 0;
}
