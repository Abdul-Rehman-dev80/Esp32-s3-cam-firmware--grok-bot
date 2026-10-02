#include "mcp_server.h"
#include <esp_log.h>

static const char* TAG = "McpServer";

McpServer& McpServer::GetInstance() {
    static McpServer instance;
    return instance;
}

void McpServer::RegisterTool(const std::string& name,
                             const std::string& description,
                             const std::vector<McpToolParam>& params,
                             McpToolHandler handler) {
    Tool tool = {name, description, params, handler};
    tools_[name] = tool;
    ESP_LOGI(TAG, "Registered MCP tool: %s", name.c_str());
}

std::string McpServer::GetServerInfo() const {
    json info = {
        {"protocolVersion", "2024-11-05"},
        {"capabilities", {{"tools", json::object()}}},
        {"serverInfo", {
            {"name", "digilog-s3cam-oled"},
            {"version", "1.0.0"}
        }}
    };
    return info.dump();
}

std::string McpServer::ListTools() const {
    json tools_json = json::array();

    for (const auto& [name, tool] : tools_) {
        json params_schema = {{"type", "object"}, {"properties", json::object()}};
        json required = json::array();

        for (const auto& param : tool.params) {
            params_schema["properties"][param.name] = {
                {"type", param.type},
                {"description", param.description}
            };
            if (param.required) {
                required.push_back(param.name);
            }
        }
        params_schema["required"] = required;

        tools_json.push_back({
            {"name", tool.name},
            {"description", tool.description},
            {"inputSchema", params_schema}
        });
    }

    return tools_json.dump();
}

std::string McpServer::CallTool(const std::string& name, const json& args) {
    auto it = tools_.find(name);
    if (it == tools_.end()) {
        return "{\"error\": \"Tool not found: " + name + "\"}";
    }

    try {
        return it->second.handler(args);
    } catch (const std::exception& e) {
        return "{\"error\": \"" + std::string(e.what()) + "\"}";
    }
}
