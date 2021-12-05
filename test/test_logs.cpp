#include <stdio.h>
#include <hash.h>
#include <logs.h>
#include <stack.h>

#include <wolfram/tree.h>
#include <wolfram/wolfram.h>
#include <wolfram/operators.h>
#include <wolfram/parse.h>

int main()
{
        printf("Hello\n");

        FILE *tex = open_tex("check.tex");
        node *rt = parse_infix("equ");
        dump_tree(rt);
        
        tex_msg(tex, "Найдем производную функции:\n");
//        $(tex_tree(tex, rt);)
        //$(node *cp = copy_tree(rt);)
        $(node *dt = diff_tree(tex, rt);)
        $(dump_tree(dt);)
//        $(tex_tree(tex, dt);)
        $(dt = optimize_tree(dt);)
        $(dump_tree(dt);)
 //       stack stk = {0};
 //       construct_stack(&stk);
 //       $(dt = replace_nodes(dt, &stk);)

        /*
        $(dump_tree(dt);)
        $(dt = optimize_tree(dt);)
        $(dump_tree(dt);)
        $(dt = optimize_tree(dt);)
        $(dump_tree(dt);)
        $(dt = optimize_tree(dt);)
        $(dump_tree(dt);)
        $(dt = optimize_tree(dt);)
        $(dump_tree(dt);)
        $(dt = optimize_tree(dt);)
        $(dump_tree(dt);)
        $(dt = optimize_tree(dt);)
        */

        //printf("%p\n", cp);
        $(dump_tree(dt);)

        tex_msg(tex, "После элементарных преобразований получаем:\n");
 //       $(tex_tree(tex, dt);)

        $(tex_tree_start(tex);)
        $(tex_tree(tex, dt);)
        $(tex_tree_end(tex);)

        tex_msg(tex, "После :\n");
        $(tex_big_tree(tex, dt);)
        close_tex(tex);
        compile_tex("check.tex");

        free_tree(dt);
        free_tree(rt);
        return 0;
}
