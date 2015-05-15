#include "bf/generator.h"

#include <fstream>

int main(int argc, char **argv) {
    bf::generator bfg;

    // Helper function to read ascii int from the input
    auto read_int = [&bfg](bf::var &var) {
        auto input = bfg.new_var("input");
        auto ascii_zero = bfg.new_var("ascii_zero", '0');
        input->read_input();
        bfg.while_begin(*input);
        {
            auto is_number = bfg.new_var("is_number");
            is_number->copy(*input);
            is_number->greater_equal(*ascii_zero);
            bfg.if_begin(*is_number);
            {
                // Add input to variable
                var.multiply(10);
                input->subtract('0'); // ascii to int
                var.add(*input);
                // Read next char
                input->read_input();
            }
            bfg.if_end(*is_number);

            auto no_number = bfg.new_var("no_number");
            no_number->bool_not(*is_number);
            bfg.if_begin(*no_number);
            {
                // End input cols
                input->set(0);
            }
            bfg.if_end(*no_number);
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
    i->copy(*rows);
    bfg.while_begin(*i);
    {
        // Just print an X for the first column
        bfg.print("X");

        // Column position, count < underscores -> print "_"
        auto count = bfg.new_var("count");

        // For each col but the first // for (j = cols - 1; j > 0; --j)
        auto j = bfg.new_var("j");
        j->copy(*cols);
        j->decrement();
        bfg.while_begin(*j);
        {
            // If count < underscores
            auto cmp_res = bfg.new_var("cmp_res");
            cmp_res->copy(*count);
            cmp_res->lower_than(*underscores);
            bfg.if_begin(*cmp_res);
            {
                // Print "_" and ++count
                bfg.print("_");
                count->increment();
            }
            bfg.if_end(*cmp_res);

            // Else (count >= underscores)
            auto not_res = bfg.new_var("not_res");
            not_res->bool_not(*cmp_res);
            bfg.if_begin(*not_res);
            {
                // Print "X" and reset count
                bfg.print("X");
                count->set(0);
            }
            bfg.if_end(*not_res);

            j->decrement();
        }
        bfg.while_end(*j);
        bfg.print("\n");

        // More underscores in the next row
        underscores->increment();
        i->decrement();
    }
    bfg.while_end(*i);

    // Print to file
    std::ofstream out("Ueb3Aufg2.bf");
    out << bfg;
    //auto minimal_code = bfg.minimal_code();
    //out << minimal_code;

    return 0;
}
