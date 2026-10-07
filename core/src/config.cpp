#include "weft/config.hpp"

#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

namespace weft {
namespace config {

using json = nlohmann::json;

namespace {
bool validateSlot(const json& s, SlotConfig& sc, std::string& err) {
    if (!s.contains("id") || !s["id"].is_string() || s["id"].get<std::string>().empty()) {
        err = "chain slot missing string 'id'";
        return false;
    }
    if (!s.contains("plugin") || !s["plugin"].is_string()) {
        err = "slot '" + sc.id + "' missing string 'plugin'";
        return false;
    }
    sc.pluginPath = s["plugin"].get<std::string>();
    if (s.contains("plugin_id") && s["plugin_id"].is_string())
        sc.pluginId = s["plugin_id"].get<std::string>();
    if (s.contains("enabled") && s["enabled"].is_boolean())
        sc.enabled = s["enabled"].get<bool>();
    if (s.contains("params")) {
        if (!s["params"].is_object()) {
            err = "slot '" + sc.id + "' 'params' must be an object";
            return false;
        }
        for (auto it = s["params"].begin(); it != s["params"].end(); ++it) {
            if (!it.value().is_number()) {
                err = "slot '" + sc.id + "' param '" + it.key() + "' must be a number (0..1)";
                return false;
            }
            float v = it.value().get<float>();
            sc.params.emplace_back(it.key(), Param::clamp01(v));
        }
    }
    return true;
}
}  // namespace

bool parse(const std::string& jsonText, ChainConfig& out, std::string* err) {
    auto fail = [&](const std::string& m) { if (err) *err = m; return false; };
    json j;
    try {
        j = json::parse(jsonText);
    } catch (const json::exception& e) {
        return fail(std::string("invalid JSON: ") + e.what());
    }
    if (!j.is_object()) return fail("root must be a JSON object");
    ChainConfig cfg;
    if (j.contains("sample_rate") && j["sample_rate"].is_number_integer())
        cfg.sampleRate = j["sample_rate"].get<int>();
    if (j.contains("block_size") && j["block_size"].is_number_integer())
        cfg.blockSize = j["block_size"].get<int>();
    if (!j.contains("chain") || !j["chain"].is_array())
        return fail("missing array 'chain'");
    std::vector<std::string> seen;
    for (const auto& s : j["chain"]) {
        if (!s.is_object()) return fail("chain entries must be objects");
        SlotConfig sc;
        std::string e;
        // id is read inside validateSlot; give it a temp for error text
        if (s.contains("id") && s["id"].is_string()) sc.id = s["id"].get<std::string>();
        if (!validateSlot(s, sc, e)) return fail(e);
        if (std::find(seen.begin(), seen.end(), sc.id) != seen.end())
            return fail("duplicate slot id '" + sc.id + "'");
        seen.push_back(sc.id);
        cfg.slots.push_back(std::move(sc));
    }
    out = std::move(cfg);
    return true;
}

bool parseFile(const std::string& path, ChainConfig& out, std::string* err) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        if (err) *err = "cannot open '" + path + "'";
        return false;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return parse(ss.str(), out, err);
}

std::string dump(const ChainConfig& cfg) {
    json j;
    j["sample_rate"] = cfg.sampleRate;
    j["block_size"] = cfg.blockSize;
    j["chain"] = json::array();
    for (const auto& s : cfg.slots) {
        json slot;
        slot["id"] = s.id;
        slot["plugin"] = s.pluginPath;
        if (!s.pluginId.empty()) slot["plugin_id"] = s.pluginId;
        slot["enabled"] = s.enabled;
        json p = json::object();
        for (const auto& kv : s.params) p[kv.first] = kv.second;
        if (!p.empty()) slot["params"] = p;
        j["chain"].push_back(slot);
    }
    return j.dump(2);
}

ChainConfig merge(const ChainConfig& base, const ChainConfig& overrideCfg) {
    ChainConfig out = base;
    out.sampleRate = overrideCfg.sampleRate > 0 ? overrideCfg.sampleRate : base.sampleRate;
    out.blockSize = overrideCfg.blockSize > 0 ? overrideCfg.blockSize : base.blockSize;

    // Build result slot list: keep base order, apply overrides by id, add new.
    std::vector<SlotConfig> result;
    result.reserve(base.slots.size() + overrideCfg.slots.size());
    for (const auto& bs : base.slots) {
        SlotConfig s = bs;
        for (const auto& os : overrideCfg.slots) {
            if (os.id == bs.id) {
                s = os;
                // merge param maps: override wins per-param, base extras kept
                s.params.clear();
                for (const auto& kv : bs.params)
                    if (!os.hasParam(kv.first)) s.params.push_back(kv);
                for (const auto& kv : os.params) s.params.push_back(kv);
                break;
            }
        }
        result.push_back(std::move(s));
    }
    for (const auto& os : overrideCfg.slots) {
        if (std::find_if(result.begin(), result.end(),
                         [&](const SlotConfig& r) { return r.id == os.id; }) == result.end())
            result.push_back(os);
    }
    out.slots = std::move(result);
    return out;
}

}  // namespace config
}  // namespace weft
