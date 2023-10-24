#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>

struct Model
{
    class Header 
    {
        std::string m_name;
        std::vector<Header> m_children;
    public:
        explicit Header(const std::string& name) : m_name(name) {}

        Header(const Header&) = delete;
        Header& operator=(const Header&) = delete;

        Header(Header&&) = default;
        Header& operator=(Header&&) = default;

        const std::string& name() const { return m_name; }

        void emplaceChild(const std::string& name) { m_children.emplace_back(name); }
        const std::vector<Header>& children() const { return m_children; }
        bool isEmpty() const { return m_children.empty(); }

        std::optional<std::reference_wrapper<Header>> getChildLeaf();
    };

    class Module 
    {
        std::string m_name;
        std::vector<Header> m_headers;

    public:
        explicit Module(const std::string& name) : m_name(name) {};
        Module(const Module&) = delete;
        Module& operator=(const Module&) = delete;
        Module(Module&&) = default;
        Module& operator=(Module&&) = default;

        void addHeader(const std::string& header) { m_headers.emplace_back(header); }
        bool isEmpty() const { return m_headers.empty(); }

        const std::string& name() const { return m_name; }
        const std::vector<Header>& headers() const { return m_headers; }

        void insertHeader(int level, const std::string& headerName);

        //std::optional<std::reference_wrapper<Header>> lastHeader();
        // std::optional<std::reference_wrapper<Header>> getLeafHeader(int level);
    };

    class Project
    {
        std::string m_name;
        std::map<std::string, Module> m_modules;

    public:
        explicit Project(std::string name) : m_name(name) {}
        Project(const Project&) = delete;
        Project& operator=(const Project&) = delete;
        Project(Project&&) = default;
        Project& operator=(Project&&) = default;

        Module& addModule(const std::string& moduleName);
        Module& getModule(const std::string& moduleName);

        const std::string& name() const { return m_name; }

        template <typename Predicate>
        Module* findModule(Predicate predicate)   // predicate: (name, module) -> bool
        {
            for (auto& [name, module] : m_modules)
                if (predicate(name, module))
                    return &module;
            return nullptr;
        }

        bool isEmpty() const;

        const std::map<std::string, Module>& modules() const { return m_modules; }
    };


    using ProjectId = int;
    std::map<ProjectId, Project> m_projects;

    Project& addProject(ProjectId projectId, std::string projectName);;
    Project& getProject(ProjectId projectId);

    void purgeEmpties();

    const std::map<ProjectId, Project>& projects() const { return m_projects; }
};


