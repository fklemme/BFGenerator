#include "bfg.h"
#include <fstream>

int main(int argc, char** argv) {
    bfg::brainfuck bf;

    // Read input
    auto cols = bf.new_var("cols");
    auto rows = bf.new_var("rows");
    {
        auto in = bf.new_var("in");
        auto zero = bf.new_var("zero", '0');
        // Read cols
        //bf.print("Cols? ");
        in->input();
        bf.while_begin(*in);
        {
            auto res = bf.new_var("res");
            in->copy_to(*res);
            res->greater_equal(*zero);
            bf.if_begin(*res);
            {
                // Add input to cols
                cols->mult(10);
                in->sub('0'); // ascii to int
                in->add_to(*cols);
                // Read next char
                in->input();
            }
            bf.if_end(*res);

            auto not_res = bf.new_var("not_res");
            not_res->not(*res);
            bf.if_begin(*not_res);
            {
                // End input cols
                in->set(0);
            }
            bf.if_end(*not_res);
        }
        bf.while_end(*in);

        // Read rows
        //bf.print("Rows? ");
        in->input();
        bf.while_begin(*in);
        {
            auto res = bf.new_var("res");
            in->copy_to(*res);
            res->greater_equal(*zero);
            bf.if_begin(*res);
            {
                // Add input to rows
                rows->mult(10);
                in->sub('0'); // ascii to int
                in->add_to(*rows);
                // Read next char
                in->input();
            }
            bf.if_end(*res);

            auto not_res = bf.new_var("not_res");
            not_res->not(*res);
            bf.if_begin(*not_res);
            {
                // End input cols
                in->set(0);
            }
            bf.if_end(*not_res);
        }
        bf.while_end(*in);
    }
    //bf.print("\n");

    // ------------------------------------------------------------------------

    // Adjacent underscores in the current row
    auto underscores = bf.new_var("underscores", 1);

    // For each row // for (i = row; i > 0; --i)
    auto i = bf.new_var("i");
    rows->copy_to(*i);
    bf.while_begin(*i);
    {
        // Just print an X for the first column
        bf.print("X");

        // Column position, count < underscores -> print "_"
        auto count = bf.new_var("count", 0);

        // For each col but the first // for (j = cols - 1; j > 0; --j)
        auto j = bf.new_var("j");
        cols->copy_to(*j);
        j->dec();
        bf.while_begin(*j);
        {
            // If count < underscores
            auto res = bf.new_var("res");
            count->copy_to(*res);
            res->lower_than(*underscores);
            bf.if_begin(*res);
            {
                // Print "_" and ++count
                bf.print("_");
                count->inc();
            }
            bf.if_end(*res);

            // Else (count >= underscores)
            auto not_res = bf.new_var("not_res");
            not_res->not(*res);
            bf.if_begin(*not_res);
            {
                // Print "X" and reset count
                bf.print("X");
                count->set(0);
            }
            bf.if_end(*not_res);

            j->dec();
        }
        bf.while_end(*j);
        bf.print("\n");

        // More underscores in the next row
        underscores->inc();
        i->dec();
    }
    bf.while_end(*i);

    // Print to file
    std::ofstream out("Ueb3Aufg2.bf");
    //out << bf;
    auto minimal_code = bf.minimal_code();
    out << minimal_code;

    return 0;
}