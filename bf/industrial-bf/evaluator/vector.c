// SAFETY: capacity is always greater than or equal to length
struct vector {
        char *ptr;
        unsigned long capacity;
        unsigned long length;
};

struct vector vector_create(unsigned long capacity) {
        struct vector vec;
        if (capacity) {
                vec.ptr = safe_malloc(capacity);
        } else {
                vec.ptr = 0;
        }
        vec.capacity = capacity;
        vec.length = 0;
        return vec;
}

void vector_extend(struct vector *vec, unsigned long new_capacity) {
        if (new_capacity < vec->capacity) return;

        vec->capacity = round_up_to_power_of_two(new_capacity);
        vec->ptr = safe_realloc(vec->ptr, vec->capacity);
}

void vector_push(struct vector *vec, char val) {
        vec->length++;
        vector_extend(vec, vec->length);
        *(vec->ptr+(vec->length-1)) = val;
}

void vector_truncate(struct vector *vec, long new_length) {
        vec->length = new_length;
}

void vector_pop(struct vector *vec, long ind) {
        vec->length--;
        for (long i = ind; i < vec->length; i++) {
                *(vec->ptr+i) = *(vec->ptr+i+1);
        }
}

void vector_drop(struct vector *vec) {
        free(vec->ptr);
}

void *vector_unwrap(struct vector *vec) {
        safe_realloc(vec->ptr, vec->length);
        return vec->ptr;
}
