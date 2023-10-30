#include "msvcParser.h"
#include "error.h"
#include <regex>
#include <fstream>
#include <cassert>

#include "model.h"
#include "config.h"

std::optional<std::pair<MsvcParser::ProjectId, std::string>> MsvcParser::splitProjectLine(const std::string& line)
{
    // Examples: 
    // - "1>------ Build started: Project: test, Configuration: Debug x64 ------"
    // - "10> some_module.cpp"
    // - "10> Note: including file: C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.36.32532\include\typeinfo"
    // - "10> C:\path\unit.cpp(58,26): warning C4099: something"
    static const std::regex moduleLineRegex(R"(\s*(\d+)>\s*(.+))");
    std::smatch match;
    if (!std::regex_match(line, match, moduleLineRegex))
        return std::nullopt;

    return std::make_pair(std::stoi(match[1]), match[2]);
}

std::optional<std::string> MsvcParser::extractNewProjectName(const std::string& projectLine)
{
    // Examples: "Project: test, Configuration: Debug x64"
    static const std::regex projectNameRegex(R"(Project:\s*(\w+))");
    std::smatch match;
    if (!std::regex_search(projectLine, match, projectNameRegex))
        return std::nullopt;
    return match[1];
}

std::optional<std::string> MsvcParser::extractModuleName(const std::string& moduleLine)
{
    // Examples: "some_module.cpp" - filenames with one or more extensions, no spaces
    static const std::regex moduleNameRegex(R"(\w+(\.\w+)+)");
    std::smatch match;
    if (!std::regex_match(moduleLine, match, moduleNameRegex))
        return std::nullopt;
    return match[0];
}

std::optional<std::string> MsvcParser::extractInculeNote(const std::string& line, bool ignoreStd)
{
    // Examples: "Note: including file: C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.36.32532\include\typeinfo"
    static const std::regex includeNoteRegex(R"(Note: including file: (.+))");
    static const std::regex stdIncludeRegex(R"(\\VC\\Tools\\MSVC\\[\.\d]+\\include\\)");
    static const std::regex crtIncludeRegex(R"(\\Windows Kits\\.+\\ucrt\\)");

    std::smatch match;
    if (!std::regex_match(line, match, includeNoteRegex))
        return std::nullopt;

    if (ignoreStd 
        && (std::regex_search(line, stdIncludeRegex) || std::regex_search(line, crtIncludeRegex)))
    {
        return std::nullopt;
    }
        
    return match[1];
}

MsvcParser::HeaderInfo MsvcParser::extractHeaderInfo(const std::string& headerName)
{
    auto itName = std::find_if_not(headerName.begin(), headerName.end(), [](char c) { return c == ' '; });
    return HeaderInfo 
    { 
        std::string(itName, headerName.end()), 
        static_cast<int>(std::distance(headerName.begin(), itName)) 
    };
}

void MsvcParser::parse(Model& model)
{
    std::ifstream compilationLog(m_config.m_inputFile);
    if (!compilationLog.is_open())
        throw Error("Error: could not open file ", m_config.m_inputFile);

    std::string line;
    while (std::getline(compilationLog, line))
    {
        auto maybeModuleLine = splitProjectLine(line);
        if (!maybeModuleLine)
            continue;

        const ProjectId projectId = maybeModuleLine->first;
        const std::string& projectLine = maybeModuleLine->second;

        // maybe, it's new project header
        if (auto newProjectName = extractNewProjectName(projectLine); newProjectName)
        {
            model.addProject(projectId, *newProjectName);
            continue;
        }

        Model::Project& project = model.getProject(projectId);

        // ... or it's module line
        if (auto newModuleName = extractModuleName(projectLine); newModuleName)
        {
            project.addModule(*newModuleName);
            m_mostRecentModule[projectId] = *newModuleName;
            continue;
        }

        // ... or it's include note
        if (auto includeNote = extractInculeNote(projectLine, m_config.m_ignoreStd); includeNote)
        {
            HeaderInfo headerInfo = extractHeaderInfo(*includeNote);
            Model::Module& unit = project.getModule(m_mostRecentModule[projectId]);
            unit.insertHeader(headerInfo.level, headerInfo.name);
            continue;
        }
    }
}
