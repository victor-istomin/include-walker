#include "ui.h"
#include "config.h"

#include "ftxui/component/captured_mouse.hpp"      // for ftxui
#include "ftxui/component/component.hpp"           // for Collapsible, Renderer, Vertical
#include "ftxui/dom/elements.hpp"                  // for text, hbox, Element
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive

#include "collapsible-colorful.hpp"

class UI::MainWindow : public ftxui::ComponentBase
{
    static ftxui::Component Prompt();

public:
    MainWindow(ftxui::Component child);

    bool OnEvent(ftxui::Event event) override;
};

ftxui::Component UI::Inner(std::vector<ftxui::Component> children)
{
    ftxui::Component vlist = ftxui::Container::Vertical(std::move(children));
    return ftxui::Renderer(vlist, [vlist] {
        return ftxui::hbox({
            ftxui::text("  "),
            vlist->Render(),
            });
        });
}

ftxui::Component UI::Empty()
{
    return std::make_shared<ftxui::ComponentBase>();
}

ftxui::Component UI::headerToComponent(const Model::Header& header)
{
    if (header.isLeaf())
    {
        // no children, just text
        auto leafComponent = ftxui::Renderer(
            [name = header.name(),
            hasCycle = header.hasCycle(),
            isCycle = header.isCycle(),
            isMatch = header.isMatched()](bool focused)
            {
                ftxui::Element element = ftxui::text(name);
                if (focused)
                    element = element | ftxui::inverted | ftxui::focus;
                if (hasCycle)
                    element = element | ftxui::color(k_colorHasCycle);
                if (isCycle)
                    element = element | ftxui::color(k_colorIsCycle);
                if (isMatch)
                    element = element | ftxui::color(k_colorIsMatch);     // #todo: highlight substring only

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
    if (header.isMatched())
        color = k_colorIsMatch;    // #todo: highlight substring only

    bool autoExpand = m_config.m_autoExpand && (header.hasCycle() || header.isCycle())
        || header.isMatched() || header.hasMatch();
    return CollapsibleColorful(header.name(), Inner(children), color, autoExpand);
}

ftxui::Component UI::moduleToComponent(const Model::Module& unit)
{
    ftxui::Components headers;
    headers.reserve(unit.headers().size());
    for (const Model::Header& header : unit.headers())
        headers.push_back(headerToComponent(header));

    bool autoExpand = m_config.m_autoExpand && unit.hasCycle();
    return Collapsible(unit.name(), Inner(headers), autoExpand);
}

ftxui::Component UI::projectToComponent(const Model::Project& project)
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

ftxui::Component UI::modelToComponent(const Model& solution)
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

UI::UI(const Config& c) : m_config(c)
{

}

ftxui::Component UI::make(const Model& solution)
{
    return std::make_shared<MainWindow>(modelToComponent(solution));
}

ftxui::Component UI::MainWindow::Prompt()
{
    return ftxui::Renderer([]
        {
            return ftxui::hbox({
                ftxui::text("↑↓ ⏎ ␣") | ftxui::bold,
                ftxui::text(" - navigate; "),
                ftxui::text("q") | ftxui::bold,
                ftxui::text(" - quit;"),
                });
        });
}

UI::MainWindow::MainWindow(ftxui::Component child)
{
    Add(ftxui::Container::Vertical({
        std::move(child),
        Prompt()
        }));
}

bool UI::MainWindow::OnEvent(ftxui::Event event)
{
    if (event == ftxui::Event::Character('q'))
    {
        if (auto screen = ftxui::ScreenInteractive::Active())
            screen->Exit();
        return true;
    }

    return ftxui::ComponentBase::OnEvent(event);
}
