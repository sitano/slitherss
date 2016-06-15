#include "config.hpp"

#include <cstdint>
#include <ostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

// bug clang++ 3.5 vs new gcc abi cant link against libboost build with gcc
// https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=797917
// http://stackoverflow.com/questions/34387406/boost-program-options-not-linking-correctly-under-clang
// http://developerblog.redhat.com/2015/02/05/gcc5-and-the-c11-abi/
game_config parse_command_line(const int argc, const char *const argv[]) {
    game_config config;

    po::options_description generic("Generic options");
    generic.add_options()
            ("help,h", po::bool_switch(&config.help), "print help message")
            ("verbose,v", po::bool_switch(&config.verbose), "set verbose output")
            ("version", po::bool_switch(&config.version), "show version information")
            ("port,p", po::value<uint16_t>(&config.port)->default_value(config.port), "bind port")
           ;

    po::options_description conf("Configuration");
    conf.add_options()
            ("bots,b", po::value<uint16_t>(&config.world.bots)->default_value(config.world.bots), "spawn bots on startup")
            ("avg_len", po::value<uint16_t>(&config.world.snake_average_length)->default_value(config.world.snake_average_length), "init snake average length")
            ("min_len", po::value<uint16_t>(&config.world.snake_min_length)->default_value(config.world.snake_min_length), "init snake min length")
            ;

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(conf);

    po::variables_map vm;

    try {
        po::store(po::parse_command_line(argc, argv, cmdline_options), vm);
        po::notify(vm);
    } catch (const po::unknown_option &e) {
        std::cerr << "error: " << e.what() << '\n';
        config.help = true;
    }

    if (config.help) {
        std::cerr << "Usage: slither_server [OPTIONS]\n";
        std::cerr << cmdline_options << '\n';
        exit(1);
    }

    return config;
}
