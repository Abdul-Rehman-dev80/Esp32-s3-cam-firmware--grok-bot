#ifndef MCP_SERVER_H_
#define MCP_SERVER_H_

#include <string>
#include <functional>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct McpToolParam {
    std::string name;
    std::string type;
    std::string description;
    bool required;
};

using McpToolHandler = std::function<std::string(const json& args)>;

class McpServer {
public:
    static McpServer& GetInstance();

    void RegisterTool(const std::string& name,
                      const std::string& description,
                      const std::vector<McpToolParam>& params,
                      McpToolHandler handler);

    std::string GetServerInfo() const;
    std::string ListTools() const;
    std::string CallTool(const std::string& name, const json& args);

private:
    McpServer() = default;

    struct Tool {
        std::string name;
        std::string description;
        std::vector<McpToolParam> params;
        McpToolHandler handler;
    };

    std::map<std::string, Tool> tools_;
};

#endif // MCP_SERVER_H_
