#include "bf/generator.h"

#include <fstream>

int main(int argc, char **argv) {
    bf::generator bfg;

    // Helper function to read ascii int from the input
    auto read_int = [&bfg](bf::var &var) {
        auto input = bfg.new_var("input");
        auto ascii_zero = bfg.new_var("ascii_zero", '0');
        input->input();
        bfg.while_begin(*input);
        {
            auto cmp_res = bfg.new_var("cmp_res");
            input->copy_to(*cmp_res);
            cmp_res->greater_equal(*ascii_zero);
            bfg.if_begin(*cmp_res);
            {
                // Add input to variable
                var.mult(10);
                input->sub('0'); // ascii to int
                input->add_to(var);
                // Read next char
                input->input();
            }
            bfg.if_end(*cmp_res);

            auto not_res = bfg.new_var("not_res");
            not_res->not_of(*cmp_res);
            bfg.if_begin(*not_res);
            {
                // End input cols
                input->set(0);
            }
            bfg.if_end(*not_res);
        }
        bfg.while_end(*input);
    };

    // Read input
    auto cols = bfg.new_var("cols");
    auto rows = bfg.new_var("rows");

    bfg.print("Cols? ");
    read_int(*cols);
    bfg.print("Rows? ");
    read_int(*rows);

    bfg.print("\n");

    // ------------------------------------------------------------------------

    // Adjacent underscores in the current row
    auto underscores = bfg.new_var("underscores", 1);

    // For each row // for (i = row; i > 0; --i)
    auto i = bfg.new_var("i");
    rows->copy_to(*i);
    bfg.while_begin(*i);
    {
        // Just print an X for the first column
        bfg.print("X");

        // Column position, count < underscores -> print "_"
        auto count = bfg.new_var("count", 0);

        // For each col but the first // for (j = cols - 1; j > 0; --j)
        auto j = bfg.new_var("j");
        cols->copy_to(*j);
        j->dec();
        bfg.while_begin(*j);
        {
            // If count < underscores
            auto cmp_res = bfg.new_var("cmp_res");
            count->copy_to(*cmp_res);
            cmp_res->lower_than(*underscores);
            bfg.if_begin(*cmp_res);
            {
                // Print "_" and ++count
                bfg.print("_");
                count->inc();
            }
            bfg.if_end(*cmp_res);

            // Else (count >= underscores)
            auto not_res = bfg.new_var("not_res");
            not_res->not_of(*cmp_res);
            bfg.if_begin(*not_res);
            {
                // Print "X" and reset count
                bfg.print("X");
                count->set(0);
            }
            bfg.if_end(*not_res);

            j->dec();
        }
        bfg.while_end(*j);
        bfg.print("\n");

        // More underscores in the next row
        underscores->inc();
        i->dec();
    }
    bfg.while_end(*i);

    // Print to file
    std::ofstream out("Ueb3Aufg2.bf");
    out << bfg;
    //auto minimal_code = bfg.minimal_code();
    //out << minimal_code;

    return 0;
}
