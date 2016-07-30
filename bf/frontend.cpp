#include "compiler.h"

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace po = boost::program_options;

int main(int argc, char **argv) {
    po::options_description desc("Options");
    desc.add_options()
        ("input-file,i",  po::value<std::string>(),                        "Set input file.")
        ("output-file,o", po::value<std::string>()->default_value("a.bf"), "Set output file.")
        ("debug,d",       "Debug information in output.")
        ("help,h",        "Print this help message.")
        ("version,v",     "Print version information.");

    po::positional_options_description pos;
    pos.add("input-file", -1);

    po::variables_map variables;
    po::store(po::command_line_parser(argc, argv).
        options(desc).positional(pos).run(), variables);
    po::notify(variables);

    if (variables.count("version")) {
        std::cout << "Brainfuck compiler, version 0.1" << std::endl;
        return 0;
    }

    if (variables.count("help") || !variables.count("input-file")) {
        std::cout << "Usage: " << argv[0] << " [options] input-file" << std::endl;
        std::cout << desc << std::endl;
        return !variables.count("help");
    }

    try {
        std::ifstream in(variables["input-file"].as<std::string>());
        const std::string source(std::istreambuf_iterator<char>(in), {});

        bf::compiler bfc;
        if (variables.count("debug"))
            bfc.enable_debug_output(true);
        const std::string bf_code = bfc.compile(source);

        std::ofstream out(variables["output-file"].as<std::string>());
        out << bf_code;
    }
    catch (const std::exception &e) {
        std::cerr << "Oops, something went wrong!\n"
                  << "Unhandled Exception: " << e.what() << std::endl;
    }

    return 0;
}
