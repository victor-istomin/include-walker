#pragma once
#include <string>

// forward declarations
namespace argh { class parser; }

struct Config
{
    bool        m_ignoreStd  = false;  // ignore std headers
    bool        m_showUsage  = false;
    bool        m_autoExpand = false;  // auto-expand cycles
    std::string m_inputFile;

    Config(argh::parser args);

    static void usageMesage();
};

