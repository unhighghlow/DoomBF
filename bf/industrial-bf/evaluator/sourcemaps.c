struct sourcemap {
        struct vector building_text;
        struct vector entries;
};

struct sourcemap_entry {
        unsigned long ind;
        char *text;
};

struct sourcemap sourcemap;

void sourcemap_init() {
        vector_init(&sourcemap.building_text, 0);
        vector_init(&sourcemap.entries, 0);
}

void sourcemap_end(unsigned long ind) {
        struct vector *bt;
        bt = &sourcemap.building_text;

        if (!bt->length) return;
        vector_push(bt, '\0');

        struct sourcemap_entry *e;
        e = safe_malloc(sizeof (struct sourcemap_entry));

        e->ind = ind;
        e->text = vector_unwrap(bt);
        vector_init(bt, 0);

        vector_push_long(&sourcemap.entries, (unsigned long)e);
}

void sourcemap_process(char chr, unsigned long ind) {
        struct vector *bt;
        bt = &sourcemap.building_text;

        if (is_whitespace(chr)) {
                if (bt->length
                 && !is_whitespace(
                        bt->ptr[bt->length-1]
                ))
                        vector_push(bt, ' ');
        } else if (is_comment(chr)) {
                vector_push(bt, chr);
        } else {
                sourcemap_end(ind);
        }
}
