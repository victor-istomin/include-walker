#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>

#include <bitset>
#include <type_traits>

template<typename T>
class EnumBits
{
private:
    static constexpr std::size_t get_max() 
    { 
        return static_cast<std::size_t>(T::MaxValue);
    } 

    typename std::underlying_type<T>::type get_value(T v) const
    {
        return static_cast<typename std::underlying_type<T>::type>(v);
    }

    std::bitset<get_max()> m_bits;

public:

    bool test(T pos) const  { return m_bits.test(get_value(pos)); }
    void set(T pos)         { m_bits.set(get_value(pos)); }
};

class Model
{
public:
    using ProjectId = int;

    enum class HeaderTraits
    {
        HasCycle,   // the header has cyclic dependency inside
        IsCycle,    // the header is the cause of cuclic dependency
        
        MaxValue
    };

    class Header 
    {
        std::string m_name;
        std::string m_normalizedName;

        std::vector<Header> m_children;
        EnumBits<HeaderTraits> m_traits;

    public:
        static char normalizeChar(char c);

        explicit Header(const std::string& name, const std::string& normalizedName, bool isCycle);

        Header(const Header&) = delete;
        Header& operator=(const Header&) = delete;

        Header(Header&&) = default;
        Header& operator=(Header&&) = default;

        const std::string& name() const { return m_name; }
        const std::string& normalizedName() const { return m_normalizedName; }

        void emplaceChild(const std::string& name, const std::string& normalizedName, bool isCycleDependency) { m_children.emplace_back(name, normalizedName, isCycleDependency); }
        const std::vector<Header>& children() const { return m_children; }
        
        bool isLeaf() const   { return m_children.empty(); }
        bool hasCycle() const { return m_traits.test(HeaderTraits::HasCycle); }
        bool isCycle() const  { return m_traits.test(HeaderTraits::IsCycle); }

        void setHasCycle() { m_traits.set(HeaderTraits::HasCycle); }
        void setIsCycle()  { m_traits.set(HeaderTraits::IsCycle); }

        Header* getLastChild();
    };

    class Module 
    {
        std::string m_name;
        std::vector<Header> m_headers;
        bool m_hasCycle = false;

    public:
        explicit Module(const std::string& name) : m_name(name) {};
        Module(const Module&) = delete;
        Module& operator=(const Module&) = delete;
        Module(Module&&) = default;
        Module& operator=(Module&&) = default;

        bool isEmpty() const { return m_headers.empty(); }
        bool hasCycle() const { return m_hasCycle; }

        const std::string& name() const { return m_name; }
        const std::vector<Header>& headers() const { return m_headers; }

        void insertHeader(int level, const std::string& headerName);
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

        bool isEmpty() const;

        const std::map<std::string, Module>& modules() const { return m_modules; }
    };

    Project& addProject(ProjectId projectId, std::string projectName);
    Project& getProject(ProjectId projectId);

    void purgeEmpties();

    const std::map<ProjectId, Project>& projects() const { return m_projects; }

private:
    std::map<ProjectId, Project> m_projects;
};


