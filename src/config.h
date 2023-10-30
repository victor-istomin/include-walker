#pragma once
#include <string>

// forward declarations
namespace argh { class parser; }

struct Config
{
    const bool        m_ignoreStd  = false;  // ignore std headers
    const bool        m_showUsage  = false;
    const bool        m_autoExpand = false;  // auto-expand cycles
    const bool        m_simplifyPath = true; // cut longest common substring from path
    const std::string m_inputFile;
    std::string m_findNormalized;            // try finding this substring, expand the tree if success

    Config(argh::parser args);

    static void showUsageMessage();

    Config(Config&) = delete;
    Config& operator=(Config&) = delete;
};

