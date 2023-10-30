#include "model.h"
#include "error.h"
#include <algorithm>
#include <cassert>
#include <locale>
#include <array>

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
    std::string normalizedName;
    normalizedName.reserve(headerName.size());
    std::transform(headerName.begin(), headerName.end(), std::back_inserter(normalizedName), &Header::normalizeChar);

    if (m_headers.empty() || level == 0)
    {
        // insert at root
        m_headers.emplace_back(headerName, normalizedName, false);
        return;
    }

    std::vector<Header*> parents;
    parents.reserve(level + 1);
    parents.push_back(&m_headers.back());
    bool isCycleDependency = parents.back()->normalizedName() == normalizedName;
    while (--level > 0)
    {
        if (Model::Header* nextLeaf = parents.back()->getLastChild(); nextLeaf)
        {
            parents.push_back(nextLeaf);
            isCycleDependency = isCycleDependency || nextLeaf->normalizedName() == normalizedName;
        }
    }

    m_hasCycle = m_hasCycle || isCycleDependency;
    parents.back()->emplaceChild(headerName, normalizedName, isCycleDependency);
    if (isCycleDependency)
        for (Header* parent : parents)
            parent->setHasCycle();
}

char Model::Header::normalizeChar(char c)
{
#if WIN32
    static const std::array<char, 256> mapping = []()
    {
        std::array<char, 256> mapping;
        for (int i = 0; i < 256; ++i)
        {
            char c = std::tolower(static_cast<char>(i));
            if (c == '/')
                c = '\\';
            mapping[i] = c;
        }
        return mapping;
    }();

    c = mapping[static_cast<unsigned char>(c)];
#endif
    return c;
}

Model::Header::Header(const std::string& name, const std::string& normalizedName, bool isCycle)
    : m_name(name)
    , m_normalizedName(normalizedName)
{
    if (isCycle)
        setIsCycle();
}

Model::Header* Model::Header::getLastChild()
{
    return m_children.empty() ? nullptr : std::addressof(m_children.back());
}
