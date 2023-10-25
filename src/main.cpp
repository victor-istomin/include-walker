#include <memory>   // for allocator, make_shared, __shared_ptr_access
#include <utility>  // for move
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Collapsible, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                  // for text, hbox, Element

#include <iostream>
#include <cassert>

#include "model.h"
#include "error.h"
#include "msvcParser.h"
#include "collapsible-colorful.hpp"

// Take a list of component, display them vertically, one column shifted to the
// right.
ftxui::Component Inner(std::vector<ftxui::Component> children) {
  ftxui::Component vlist = ftxui::Container::Vertical(std::move(children));
  return ftxui::Renderer(vlist, [vlist] {
    return ftxui::hbox({
        ftxui::text(" "),
        vlist->Render(),
    });
  });
}
 
ftxui::Component Empty() {
  return std::make_shared<ftxui::ComponentBase>();
}

static const ftxui::Color k_colorHasCycle = ftxui::Color::Yellow;
static const ftxui::Color k_colorIsCycle  = ftxui::Color::Red;

ftxui::Component headerToComponent(const Model::Header& header)
{
    if (header.isLeaf())
    {
        // no children, just text
        auto leafComponent = ftxui::Renderer(
            [name = header.name(), hasCycle = header.hasCycle(), isCycle = header.isCycle()](bool focused)
        {
            ftxui::Element element = ftxui::text(name);
            if (focused)
                element = element | ftxui::inverted | ftxui::focus;
            if (hasCycle)
                element = element | ftxui::color(k_colorHasCycle);
            if (isCycle)
                element = element | ftxui::color(k_colorIsCycle);
            return element;
        });

        return leafComponent;
    }

    // has children, collapsible
    ftxui::Components children;
    children.reserve(header.children().size());
    for (const Model::Header& child : header.children())
        children.push_back(headerToComponent(child));

    ftxui::Color color = ftxui::Color::Default;
    if (header.hasCycle())
        color = k_colorHasCycle;
    if (header.isCycle())
        color = k_colorIsCycle;

    return CollapsibleColorful(header.name(), Inner(children), color);
}

ftxui::Component moduleToComponent(const Model::Module& module)
{
    ftxui::Components headers;
    headers.reserve(module.headers().size());
    for (const Model::Header& header : module.headers())
        headers.push_back(headerToComponent(header));
    
    return Collapsible(module.name(), Inner(headers));
}

ftxui::Component projectToComponent(const Model::Project& project)
{
    ftxui::Components modules;
    for(const auto& [name, module] : project.modules())
        modules.push_back(moduleToComponent(module));

    return Collapsible(project.name(), Inner(modules));
}

ftxui::Component modelToComponent(const Model& solution)
{
    auto container = ftxui::Container::Vertical({});
    for (const auto& [_, project] : solution.projects())
        container->Add(projectToComponent(project));

    auto renderer = ftxui::Renderer(container, [=] {
        return container->Render() 
            | ftxui::vscroll_indicator | ftxui::frame | ftxui::border;
    });

    return renderer;
}

void printCycle(const Model::Header& header)
{
    if (header.isCycle())
        std::cout << " * cycle detected: " << header.name() << std::endl;

    for (const auto& child : header.children())
        printCycle(child);
}

int main(int argc, const char* argv[])
{
    if (argc < 2) 
    {
        std::cout << "Usage: " << argv[0] << " <compilation log file>" << std::endl;
        return 0;
    } 
    
    std::string compilationLogFile = argv[1];
    std::cout << "Parsing compilation log file: " << compilationLogFile << std::endl;

    Model model;
    MsvcParser msvcParser = MsvcParser(model);

    try
    {
        msvcParser.parse(compilationLogFile);
        model.purgeEmpties();

        for (const auto& [_, project] : model.projects())
            for (const auto& [_, module] : project.modules())
                for (const auto& header : module.headers())
                    printCycle(header);
    }

    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    auto mine = modelToComponent(model);
    auto screen = ftxui::ScreenInteractive::FitComponent();
    screen.TrackMouse(false);
    screen.Loop(mine);
}