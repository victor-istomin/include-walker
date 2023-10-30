#include "config.h"
#include "argh.h"
#include "model.h"
#include <iostream>


void Config::showUsageMessage()
{
    std::cout << "Usage: "
              << "  include-walker <compilation log file> [--no-std] [--auto-expand] [--find='substring']" << std::endl;
}

Config::Config(argh::parser args)
    : m_ignoreStd(args["--no-std"])
    , m_showUsage(args[{"--help", "--usage"}] || args.size() < 2)
    , m_autoExpand(args["--auto-expand"])
    , m_simplifyPath(args["--simplify"])
    , m_inputFile(args[1]/*first positional*/)
    , m_findNormalized(args("--find").str())
{
    std::cout << "Parsing compilation log file: " << m_inputFile << std::endl;
    if (m_ignoreStd)
        std::cout << " + ignore std headers" << std::endl;
    if (m_autoExpand)
        std::cout << " + auto-expand tree with loops" << std::endl;
    if (m_simplifyPath)
        std::cout << " + simplify path" << std::endl;
    if (!m_findNormalized.empty())
        std::cout << " + search for: '" << m_findNormalized << "'" << std::endl;

    std::transform(m_findNormalized.begin(), m_findNormalized.end(), m_findNormalized.begin(), &Model::Header::normalizeChar);
}

