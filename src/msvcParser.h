#pragma once
#include <string>
#include <map>
#include <optional>

#include "model.h"

class MsvcParser
{
    using ProjectId = Model::ProjectId;
    using HeaderLevel = int;
    struct HeaderInfo
    {
        std::string name;
        HeaderLevel level = -1;
    };

    Model& m_model;
    std::map<ProjectId, std::string> m_mostRecentModule;

    static std::optional<std::pair<ProjectId, std::string>> splitProjectLine(const std::string& line);

    static std::optional<std::string> extractNewProjectName(const std::string& projectLine);

    static std::optional<std::string> extractModuleName(const std::string& moduleLine);

    static std::optional<std::string> extractInculeNote(const std::string& line);

    static HeaderInfo extractHeaderInfo(const std::string& headerName);

public:
    MsvcParser(Model& model) : m_model(model) {}

    void parse(const std::string& logFileName);
};


