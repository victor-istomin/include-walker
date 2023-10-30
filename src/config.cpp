#include "config.h"
#include "argh.h"
#include <iostream>


void Config::usageMesage()
{
    std::cout << "Usage: "
              << "  include-walker <compilation log file> [--no-std] [--auto-expand]" << std::endl;
}

Config::Config(argh::parser args)
    : m_ignoreStd(args["--no-std"])
    , m_showUsage(args[{"--help", "--usage"}] || args.size() < 2)
    , m_autoExpand(args["--auto-expand"])
    , m_inputFile(args[1]/*first positional*/)
{
    std::cout << "Parsing compilation log file: " << m_inputFile << std::endl;
    if (m_ignoreStd)
        std::cout << " + ignore std headers" << std::endl;
    if (m_autoExpand)
        std::cout << " + auto-expand tree with loops" << std::endl;
}

