#include <memory>   // for allocator, make_shared, __shared_ptr_access
#include <utility>  // for move
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Collapsible, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                  // for text, hbox, Element

#include <iostream>
#include <cassert>

#include "argh.h"

#include "model.h"
#include "error.h"
#include "msvcParser.h"
#include "collapsible-colorful.hpp"
#include "config.h"


static const ftxui::Color k_colorHasCycle = ftxui::Color::Yellow;
static const ftxui::Color k_colorIsCycle = ftxui::Color::Red;


class UI
{
    const Config& m_config;

    // Take a list of component, display them vertically, one column shifted to the
    // right.
    static ftxui::Component Inner(std::vector<ftxui::Component> children) {
        ftxui::Component vlist = ftxui::Container::Vertical(std::move(children));
        return ftxui::Renderer(vlist, [vlist] {
            return ftxui::hbox({
                ftxui::text(" "),
                vlist->Render(),
                });
            });
    }

    static ftxui::Component Empty() {
        return std::make_shared<ftxui::ComponentBase>();
    }

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

        bool autoExpand = m_config.m_autoExpand && (header.hasCycle() || header.isCycle());
        return CollapsibleColorful(header.name(), Inner(children), color, autoExpand);
    }

    ftxui::Component moduleToComponent(const Model::Module& unit)
    {
        ftxui::Components headers;
        headers.reserve(unit.headers().size());
        for (const Model::Header& header : unit.headers())
            headers.push_back(headerToComponent(header));

        bool autoExpand = m_config.m_autoExpand&& unit.hasCycle();
        return Collapsible(unit.name(), Inner(headers), autoExpand);
    }

    ftxui::Component projectToComponent(const Model::Project& project)
    {
        ftxui::Components modules;
        bool autoExpand = false;
        for (const auto& [name, unit] : project.modules())
        {
            autoExpand = autoExpand || (m_config.m_autoExpand && unit.hasCycle());
            modules.push_back(moduleToComponent(unit));
        }

        return Collapsible(project.name(), Inner(modules), autoExpand);
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

public:
    UI(const Config& c) : m_config(c) {}

    ftxui::Component make(const Model& solution)
    {
        return modelToComponent(solution);
    }
};

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
        config.usageMesage();
        return 1;
    }
    
    std::string compilationLogFile = argv[1];

    Model model;
    MsvcParser msvcParser = MsvcParser(config);

    try
    {
        msvcParser.parse(model);
        model.purgeEmpties();

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