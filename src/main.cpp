#include <memory>   // for allocator, make_shared, __shared_ptr_access
#include <utility>  // for move
 
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive

#include <iostream>
#include <cassert>

#include "argh.h"

#include "model.h"
#include "error.h"
#include "msvcParser.h"
#include "config.h"
#include "ui.h"


void printCycle(const Model::Header& header)
{
    if (header.isCycle())
        std::cout << " * cycle detected: " << header.name() << std::endl;

    for (const auto& child : header.children())
        printCycle(child);
}

int main(int argc, const char* argv[])
{
    Config config = argh::parser(argv);
    if (config.m_showUsage)
    {
        config.showUsageMessage();
        return 1;
    }
    
    std::string compilationLogFile = argv[1];

    Model model = Model(config);
    MsvcParser msvcParser = MsvcParser(config);

    try
    {
        msvcParser.parse(model);
        model.purgeEmpties();
        if (config.m_simplifyPath)
            model.simplifyPath();

        for (const auto& [_, project] : model.projects())
            for (const auto& [_, unit] : project.modules())
                for (const auto& header : unit.headers())
                    printCycle(header);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    UI ui = UI(config);
    auto mine = ui.make(model);
    auto screen = ftxui::ScreenInteractive::FitComponent();
    screen.TrackMouse(false);
    screen.Loop(mine);

}