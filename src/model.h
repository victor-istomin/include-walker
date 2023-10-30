#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>

#include <bitset>
#include <type_traits>

struct Config;

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
        HasCycle,    // the header has cyclic dependency inside
        IsCycle,     // the header is the cause of cyclic dependency
        IsMatched,   // was matched by the search
        HasMatch,    // has mathced search substring in children list

        MaxValue
    };

    class Header 
    {
    public:
        struct MatchInfo
        {
            std::size_t m_begin = 0;
            std::size_t m_end   = 0;
        };

        static char normalizeChar(char c);

        explicit Header(const std::string& name, const std::string& normalizedName, bool isCycle);

        Header(const Header&) = delete;
        Header& operator=(const Header&) = delete;

        Header(Header&&) = default;
        Header& operator=(Header&&) = default;

        const std::string& name() const { return m_name; }
        const std::string& normalizedName() const { return m_normalizedName; }

        Header& emplaceChild(const std::string& name, const std::string& normalizedName, bool isCycleDependency);
        const std::vector<Header>& children() const { return m_children; }
        
        bool isLeaf() const   { return m_children.empty(); }
        bool hasCycle() const { return m_traits.test(HeaderTraits::HasCycle); }
        bool isCycle() const  { return m_traits.test(HeaderTraits::IsCycle); }
        bool hasMatch() const { return m_traits.test(HeaderTraits::HasMatch); }
        bool isMatched() const { return m_traits.test(HeaderTraits::IsMatched); }

        void setHasCycle() { m_traits.set(HeaderTraits::HasCycle); }
        void setHasMatch() { m_traits.set(HeaderTraits::HasMatch); }
        void setIsCycle()  { m_traits.set(HeaderTraits::IsCycle); }
        bool tryMatch(const std::string& normalizedString);

        Header* getLastChild();
        MatchInfo getMatch() const { return m_match; }
        void simplifyPath(size_t prefixSize);

    private:
        std::string m_name;
        std::string m_normalizedName;

        std::vector<Header> m_children;
        EnumBits<HeaderTraits> m_traits;
        MatchInfo m_match;
    };

    class Module 
    {
        std::string m_name;
        std::vector<Header> m_headers;
        const Config& m_config; 
        bool m_hasCycle = false;
        std::string m_longestPrefix;

    public:
        explicit Module(const std::string& name, const Config& config) : m_name(name), m_config(config) {};
        Module(const Module&) = delete;
        Module& operator=(const Module&) = delete;
        Module(Module&&) = default;
        Module& operator=(Module&&) = default;

        bool isEmpty() const { return m_headers.empty(); }
        bool hasCycle() const { return m_hasCycle; }

        const std::string& name() const { return m_name; }
        const std::vector<Header>& headers() const { return m_headers; }

        void insertHeader(int level, const std::string& headerName);
        void updateLongestPrefix(const std::string& normalizedName);
        void simplifyPath();
    };

    class Project
    {
        std::string m_name;
        std::map<std::string, Module> m_modules;
        const Config& m_config;

    public:
        explicit Project(std::string name, const Config& config) : m_name(name), m_config(config) {}
        Project(const Project&) = delete;
        Project& operator=(const Project&) = delete;
        Project(Project&&) = default;
        Project& operator=(Project&&) = default;

        Module& addModule(const std::string& moduleName);
        Module& getModule(const std::string& moduleName);
        void simplifyPath();

        const std::string& name() const { return m_name; }

        bool isEmpty() const;

        const std::map<std::string, Module>& modules() const { return m_modules; }
    };

    explicit Model(const Config& config) : m_config(config) {}

    Project& addProject(ProjectId projectId, std::string projectName);
    Project& getProject(ProjectId projectId);

    void purgeEmpties();
    void simplifyPath();

    const std::map<ProjectId, Project>& projects() const { return m_projects; }

private:
    std::map<ProjectId, Project> m_projects;
    const Config& m_config;
};


