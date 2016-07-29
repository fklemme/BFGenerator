#include "compiler.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main(int argc, char **argv) {
    if (argc != 2) return 1;

    if (std::string(argv[1]) == "--version") {
        std::cout << "Brainfuck compiler, version 0.1" << std::endl;
        return 0;
    }

    std::ifstream in(argv[1]);
    const std::string source(std::istreambuf_iterator<char>(in), {});

    bf::compiler bfc;
    const std::string bf_code = bfc.compile(source);

    std::ofstream out("a.bf");
    out << bf_code;

    return 0;
}
