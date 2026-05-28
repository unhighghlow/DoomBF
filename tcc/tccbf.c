#include "tcc.h"

#define _bf_fwrite_str(str, fd) fwrite(str, 1, strlen(str), fd)

ST_FUNC int bf_output_file(TCCState *s1, const char *filename)
{
    int fd, mode, file_type, reloc_val;
    FILE *fp;
    char chr;

    file_type = s1->output_type;
    if (file_type == TCC_OUTPUT_OBJ)
        mode = 0666;
    else
        mode = 0777;
    unlink(filename);
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode);
    if (fd < 0 || (fp = fdopen(fd, "wb")) == NULL) {
        tcc_error_noabort("could not write '%s: %s'", filename, strerror(errno));
        return -1;
    }

    tcc_add_runtime(s1);
    resolve_common_syms(s1);

    text_section->sh_size = text_section->data_offset;
    _bf_fwrite_str(
        "+>+["
        "[-]<[>+<-]>"
        "\n"
    ,fp);

    for (int ind = 0; ind < text_section->sh_size; ind++) {
        chr = text_section->data[ind];
        if (chr) {
            fwrite(&chr, 1, 1, fp);
            continue;
        }
        /* Relocatable value */
        _bf_fwrite_str("reloc", fp);
        ind++;
        reloc_val = read32le(text_section->data + ind);
        ind += 3;
        while (reloc_val--) {
            fwrite("+", 1, 1, fp);
        }
    }
    _bf_fwrite_str(
        "]"
        "\n"
    ,fp);
    return 0;
}
