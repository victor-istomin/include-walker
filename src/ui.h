#pragma once

#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "model.h"

static const ftxui::Color k_colorHasCycle = ftxui::Color::Yellow;
static const ftxui::Color k_colorIsCycle = ftxui::Color::Red;
static const ftxui::Color k_colorIsMatch = ftxui::Color::Blue;

struct Config;

class UI
{
    const Config& m_config;

    class MainWindow;
    static ftxui::Component Inner(std::vector<ftxui::Component> children);
    static ftxui::Component Empty();

    ftxui::Component headerToComponent(const Model::Header& header);
    ftxui::Component moduleToComponent(const Model::Module& unit);
    ftxui::Component projectToComponent(const Model::Project& project);
    ftxui::Component modelToComponent(const Model& solution);

public:
    UI(const Config& c);

    ftxui::Component make(const Model& solution);
};


