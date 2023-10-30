#pragma once
#include <string>
#include <map>
#include <optional>


struct Config;
class Model;

class MsvcParser
{
    using ProjectId = int;
    using HeaderLevel = int;
    struct HeaderInfo
    {
        std::string name;
        HeaderLevel level = -1;
    };

    const Config& m_config;
    std::map<ProjectId, std::string> m_mostRecentModule;

    static std::optional<std::pair<ProjectId, std::string>> splitProjectLine(const std::string& line);

    static std::optional<std::string> extractNewProjectName(const std::string& projectLine);

    static std::optional<std::string> extractModuleName(const std::string& moduleLine);

    static std::optional<std::string> extractInculeNote(const std::string& line, bool ignoreStd);

    static HeaderInfo extractHeaderInfo(const std::string& headerName);

public:
    MsvcParser(const Config& config) : m_config(config) {}

    void parse(Model& model);
};


