/* bf2for -- Convert brainfuck programs to FORTRAN.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void  usage(char*);
FILE *safe_fopen(char*, char*);
void  out_str(char*);

FILE *in_fp;
FILE *out_fp;

#define oc(s) for (i = 0; i < indent; i++) {out_str("  ");}; out_str(s "\n")
#define o(s) oc(s); break;

int main(int argc, char *argv[]) {
        char chr;
        int indent = 1, i;

        if (argc < 2 || argc > 3) usage(argv[0]);

        in_fp = safe_fopen(argv[1], "rb");
        out_fp = 0;
        if (argc > 2) {
                out_fp = safe_fopen(argv[2], "w");
        }

        out_str(
                "program brainfuck_compiled\n"
                "  implicit none\n"
                "  integer :: a(30000)\n"
                "  integer :: p\n"
                "  p = 0\n"
        );
        while (fread(&chr, 1, 1, in_fp)) {
                switch (chr) {
                case '+': o("a(p:p) = a(p:p) + 1")
                case '-': o("a(p:p) = a(p:p) - 1")
                case '<': o("p = p - 1")
                case '>': o("p = p + 1")
                case '.': o("print \"(A,$)\", CHAR(a(p))")
                case ',': o("!in")
                case '[': oc("do while (a(p) /= 0)"); indent++; break;
                case ']': indent--; o("end do")
                default: break;
                }
        }
        out_str(
                "end program\n"
        );

        fclose(in_fp);
        if (out_fp) fclose(out_fp);

        return 0;
}

void out_str(char *s) {
        if (!out_fp) {
                printf("%s", s);
                return;
        }
        fwrite(s, 1, strlen(s), out_fp);
}

FILE *safe_fopen(char *filename, char *mode) {
        FILE *out;

        out = fopen(filename, mode);
        if (!out) {
                perror("cannot open file");
                exit(1);
        }
        return out;
}

void usage(char *exec) {
        printf("Usage: %s PROGRAM [OUT]\n", exec);
        printf("Convert brainfuck programs to FORTRAN.\n");
        printf("\n");
        printf("With no OUT, or when OUT is -, write to standard output.\n");
        exit(2);
}
