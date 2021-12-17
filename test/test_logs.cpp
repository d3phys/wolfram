#include <stdio.h>
#include <hash.h>
#include <logs.h>
#include <stack.h>
#include <errno.h>
#include <string.h>

#include <wolfram/tree.h>
#include <wolfram/wolfram.h>
#include <wolfram/operators.h>
#include <wolfram/parse.h>

int main(int argc, char *argv[])
{
        if (argc < 2 || argc > 3) {
                fprintf(stderr, "There must be 1-2 args\n");
                return EXIT_FAILURE;
        }

        FILE *tex = open_tex("check.tex");

        if (argc == 3) {
                if (strlen(argv[2]) != 1) {
                        fprintf(stderr, "It should be 1 letter variable %s", argv[2]);
                        return EXIT_FAILURE;
                }
        }

        char var = 0;
        if (argc == 3) {
                var = *argv[2];
        }

        node *rt = parse_infix(argv[1]);
        dump_tree(rt);
        
        tex_msg(tex, "Найдем производную proстейшей функции:\n");
        $(tex_tree_start(tex, 0);)
        $(tex_tree(tex, rt);)
        $(tex_tree_end(tex);)

        node *dt = nullptr;
        if (var) {
                $(dt = diff_tree(tex, rt, var);)
        } else {
                $(dt = diff_tree(tex, rt);)
        }

        $(dump_tree(dt);)
        $(dt = optimize_tree(dt);)
        $(dump_tree(dt);)
        $(dump_tree(dt);)

        tex_msg(tex, "После элементарных преобразований получаем:\n");
        $(tex_big_tree(tex, dt, 0));
        tex_msg(tex, "\\textbf{Пусть вниметельный читатель засунет свой учебник по матану себе в задницу}\n");

        close_tex(tex);
        compile_tex("check.tex");

        free_tree(dt);
        free_tree(rt);
        return 0;
}
