#include "thresholds.h"

#include <fstream>
#include <sstream>
#include <stack>

#include "circle.h"
#include "solo.h"

namespace thresholds {

Thresholds thresholds;

ServerIDType ServerType(const std::string& value) {
    if (value == "EASY") {
        return EASY;
    } else if (value == "KIDS") {
        return KIDS;
    } else if (value == "NIVAL") {
        return NIVAL;
    } else if (value == "MEDIUM") {
        return MEDIUM;
    } else if (value == "HARD") {
        return HARD;
    } else if (value == "NIGHTMARE") {
        return NIGHTMARE;
    } else if (value == "QUEST_T1") {
        return QUEST_T1;
    } else if (value == "QUEST_T2") {
        return QUEST_T2;
    } else if (value == "QUEST_T3") {
        return QUEST_T3;
    } else if (value == "QUEST_T4") {
        return QUEST_T4;
    }
    return UNDEFINED;
}

void Thresholds::LoadFromContent(std::string content) {
    std::istringstream stream(content);
    LoadFromStream(stream);
}

void Thresholds::LoadFromFile(std::string file_name) {
    std::ifstream fin(file_name);
    if (!fin) {
        throw ParseException(Format("failed to open file '%s'\n", file_name.c_str()));
    }

    LoadFromStream(fin);
}

void Thresholds::LoadFromStream(std::istream& stream) {
    std::string line;
    int line_number = 0;

    std::stack<std::shared_ptr<Node>> stack;
    auto root = std::make_shared<Node>();
    stack.push(root);

    while (std::getline(stream, line)) {
        ++line_number;

        size_t comment = line.find("//");
        if (comment != std::string::npos) {
            line = line.substr(0, comment);
        }

        line = Trim(line);
        if (line.length() == 0) {
            continue;
        }

        if (line.back() == '{') {
            std::string section_name = Trim(line.substr(0, line.length() - 1));

            auto node = std::make_shared<Node>();
            for (auto alias : Explode(section_name, "|")) {
                alias = Trim(alias);

                auto server_id = ServerType(alias);
                if (server_id != UNDEFINED) {
                    stack.top()->by_server[server_id] = node;
                } else {
                    stack.top()->children[alias] = node;
                }

                if (!stack.top()->children.empty() && !stack.top()->by_server.empty()) {
                    throw ParseException(Format("line %d: section contains a mix of subsection types: children=%d, by_server=%d", line_number, node->children.size(), node->by_server.size()));
                }
            }
            stack.push(node);
        } else if (line.back() == '}') {
            if (stack.empty()) {
                throw ParseException(Format("line %d: invalid syntax: too many closing brackets", line_number));
            }

            stack.pop();
        } else {
            size_t equals = line.find('=');

            if (equals == std::string::npos) {
                throw ParseException(Format("line %d: invalid syntax: '%s' is not a section or a key-value pair", line_number, line));
            }

            std::string key = Trim(line.substr(0, equals));

            // Parse the value.
            std::string value = Trim(line.substr(equals + 1));
            uint32_t multiplier = 1;

            if (value.back() == 'k') {
                multiplier = 1000;
                value.pop_back();
            } else if (value.back() == 'm') {
                multiplier = 1000 * 1000;
                value.pop_back();
            }

            if (!CheckInt(value)) {
                throw ParseException(Format("line %d: invalid syntax: '%s' is not a number", line_number, value));
            }

            uint32_t number = static_cast<uint32_t>(StrToInt(value)) * multiplier;

            auto& node = stack.top();

            // Parse the key type. Can be a regular key-value mapping or a server_id mapping, or a mob mapping.
            auto server = ServerType(key);
            if (server != UNDEFINED) {
                node->by_server[server] = std::make_shared<Node>(number);
            } else {
                if (CheckInt(key)) {
                    uint16_t mob_id = static_cast<uint16_t>(StrToInt(key));
                    node->mobs[mob_id] = static_cast<int>(number);
                } else {
                    node->children[key] = std::make_shared<Node>(number);
                }
            }

            if (node->children.empty() + node->by_server.empty() + node->mobs.empty() != 2) {
                throw ParseException(Format("line %d: section contains a mix of key types: children=%d, by_server=%d, mobs=%d", line_number, node->children.size(), node->by_server.size(), node->mobs.size()));
            }
        }
    }

    if (stack.size() != 1) {
        throw ParseException(Format("line %d: invalid syntax: section was not closed", line_number));
    }

    this->root = std::move(root);
    this->loaded_at = std::chrono::steady_clock::now();
}

void Thresholds::MaybeReload(std::string file_name) {
    auto now = std::chrono::steady_clock::now();

    // Reload at most once a minute.
    if (!this->root || (now - this->loaded_at) > std::chrono::minutes(1)) {
        try {
            Printf("Reloading thresholds\n");
            LoadFromFile(file_name);
        } catch (const ParseException& error) {
            Printf("Failed to reload thresholds: %s\n", error.what());
        }
    }
}

uint32_t Thresholds::Value(std::string key, const A2Unit* main_unit) const {
    const Node* node = this->Descend(this->Path(key), main_unit);
    return node ? node->number : 0;
}

const std::unordered_map<uint16_t, int>* Thresholds::Mobs(std::string key, const A2Unit* main_unit) const {
    const Node* node = this->Descend(this->Path(key), main_unit);
    return node ? &node->mobs : nullptr;
}

const Node* Thresholds::Path(std::string key) const {
    Node* node = this->root.get();

    for (auto p: Explode(key, ".")) {
        node = node->children[p].get();
        if (!node) {
            return nullptr;
        }
    }

    return node;
}

const Node* Thresholds::Descend(const Node* at, const A2Unit* main_unit) const {
    if (!at) {
        return nullptr;
    }

    // First descend by character kind.
    if (!at->children.empty()) {
        if (IsFemale(main_unit)) {
            auto ptr = at->children.find("female");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }

        if (circle::Circle(main_unit) > 0) {
            auto ptr = at->children.find("hell");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }

        if (IsGigaPlayer(main_unit)) {
            auto ptr = at->children.find("legend");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }

        if (IsPureSoloPlayer(main_unit)) {
            auto ptr = at->children.find("ironman");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }

        if (main_unit->player->deaths <= 1) {
            auto ptr = at->children.find("hardcore");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }
        
        auto ptr = at->children.find("default");
        if (ptr != at->children.end()) {
            return this->Descend(ptr->second.get(), main_unit);
        }
    }

    // Then descend by server ID.
    if (!at->by_server.empty()) {
        auto ptr = at->by_server.find(Config::ServerID);
        if (ptr == at->by_server.end()) {
            // Intentional. If settings aren't filled for the server ID, there are no thresholds.
            return nullptr;
        }

        return this->Descend(ptr->second.get(), main_unit);
    }

    return at;
}

} // namespace thresholds
