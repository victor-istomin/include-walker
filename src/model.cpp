#include "model.h"
#include "error.h"
#include "config.h"
#include <algorithm>
#include <cassert>
#include <locale>
#include <array>

Model::Module& Model::Project::addModule(const std::string& moduleName)
{
    auto [it, isInserted] = m_modules.emplace(moduleName, Module(moduleName, m_config));
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

void Model::Project::simplifyPath()
{
    for (auto& [_, unit] : m_modules)
        unit.simplifyPath();
}

bool Model::Project::isEmpty() const
{
    return m_modules.empty()
        || std::all_of(m_modules.begin(), m_modules.end(), [](const auto& nameModulePair) { return nameModulePair.second.isEmpty(); });
}

Model::Project& Model::addProject(ProjectId projectId, std::string projectName)
{
    auto [it, isInserted] = m_projects.emplace(projectId, Project(projectName, m_config));
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

void Model::simplifyPath()
{
    for (auto& [_, project] : m_projects)
        project.simplifyPath();
}

void Model::Module::insertHeader(int level, const std::string& headerName)
{
    std::string normalizedName;
    normalizedName.reserve(headerName.size());
    std::transform(headerName.begin(), headerName.end(), std::back_inserter(normalizedName), &Header::normalizeChar);
    // #todo: refactor and improve; should normalize '/../' as well as case

    if (m_config.m_simplifyPath)
        updateLongestPrefix(normalizedName);

    if (m_headers.empty() || level == 0)
    {
        // insert at root
        m_headers.emplace_back(headerName, normalizedName, false);
        m_headers.back().tryMatch(m_config.m_findNormalized);
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
    Model::Header& newHeader = parents.back()->emplaceChild(headerName, normalizedName, isCycleDependency);
    bool isMatched = newHeader.tryMatch(m_config.m_findNormalized);

    if (isCycleDependency || isMatched)
    {
        for (Header* parent : parents)
        {
            // #todo -  refactor: extract method, use Header's bit flags
            if (isCycleDependency)
                parent->setHasCycle();
            if (isMatched)
                parent->setHasMatch();
        }
    }
}

void Model::Module::updateLongestPrefix(const std::string& normalizedName)
{
    if (isEmpty())
    {
        m_longestPrefix = normalizedName;
        return;
    }

    if (m_longestPrefix.empty())
        return;

    size_t matchEnd = 0;
    for (matchEnd = 0; matchEnd < std::min(m_longestPrefix.size(), normalizedName.size()); ++matchEnd)
        if (normalizedName[matchEnd] != m_longestPrefix[matchEnd])
            break;

    m_longestPrefix.erase(matchEnd);
}

void Model::Module::simplifyPath()
{
    size_t prefixSize = m_longestPrefix.size();
    for (Header& header : m_headers)
        header.simplifyPath(prefixSize);
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

Model::Header& Model::Header::emplaceChild(const std::string& name, const std::string& normalizedName, bool isCycleDependency)
{
    m_children.emplace_back(name, normalizedName, isCycleDependency);
    return m_children.back();
}

bool Model::Header::tryMatch(const std::string& normalizedString)
{
    if (normalizedString.empty())
        return false;

    std::string::size_type matchPos = m_normalizedName.find(normalizedString);
    bool hasMatch = std::string::npos != matchPos;
    if (hasMatch)
    {
        m_match = {matchPos, matchPos + normalizedString.size()};
        m_traits.set(HeaderTraits::IsMatched);
    }

    return hasMatch;
}

Model::Header* Model::Header::getLastChild()
{
    return m_children.empty() ? nullptr : std::addressof(m_children.back());
}

void Model::Header::simplifyPath(size_t prefixSize)
{
    m_name.erase(0, prefixSize);
    for (Header& child : m_children)
        child.simplifyPath(prefixSize);
}
