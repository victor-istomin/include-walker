#include "model.h"
#include "error.h"
#include <algorithm>
#include <cassert>

Model::Module& Model::Project::addModule(const std::string& moduleName)
{
    auto [it, isInserted] = m_modules.emplace(moduleName, moduleName);
    if (!isInserted)
        throw Error("Error: module ", moduleName, " already exists");

    return it->second;
}

Model::Module& Model::Project::getModule(const std::string& moduleName)
{
    auto it = m_modules.find(moduleName);
    if (it == m_modules.end())
        throw Error("Error: module ", moduleName, " does not exist");

    return it->second;
}

bool Model::Project::isEmpty() const
{
    return m_modules.empty()
        || std::all_of(m_modules.begin(), m_modules.end(), [](const auto& nameModulePair) { return nameModulePair.second.isEmpty(); });
}

Model::Project& Model::addProject(ProjectId projectId, std::string projectName)
{
    auto [it, isInserted] = m_projects.emplace(projectId, Project(projectName));
    if (!isInserted)
        throw Error("Error: project id ", std::to_string(projectId), " already exists");

    return it->second;
}

Model::Project& Model::getProject(ProjectId projectId)
{
    auto it = m_projects.find(projectId);
    if (it == m_projects.end())
        throw Error("Error: project id ", std::to_string(projectId), " does not exist");

    return it->second;
}

void Model::purgeEmpties()
{
    for (auto itProject = m_projects.begin(); itProject != m_projects.end();)
    {
        if (itProject->second.isEmpty())
            itProject = m_projects.erase(itProject);
        else
            ++itProject;
    }
}

void Model::Module::insertHeader(int level, const std::string& headerName)
{
    if (m_headers.empty() || level == 0)
    {
        // insert at root
        m_headers.emplace_back(headerName);
        return;
    }

    std::optional<std::reference_wrapper<Model::Header>> leaf = std::ref(m_headers.back());

    auto nextLeaf = leaf->get().getChildLeaf();
    while (--level > 0 && nextLeaf.has_value())
    {
        leaf = nextLeaf;
        nextLeaf = leaf.has_value() ? leaf->get().getChildLeaf() : std::nullopt;
    }

    leaf->get().emplaceChild(headerName);
}

// std::optional<std::reference_wrapper<Model::Header>> Model::Module::getLeafHeader(int level)
// {
//     if (m_headers.empty())
//         return std::nullopt;
// 
//     if (level == 0)
//         return std::nullopt;    // level 0 is root
// 
// 
//     return leaf;
// }

std::optional<std::reference_wrapper<Model::Header>> Model::Header::getChildLeaf()
{
    if (m_children.empty())
        return std::nullopt;

    return std::ref(m_children.back());
}
