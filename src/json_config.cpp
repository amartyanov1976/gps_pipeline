#include "infra/json_config.h"
#include <cctype>
#include <iostream>
#include <algorithm>

JsonConfig::JsonConfig() = default;

JsonConfig::JsonConfig(const std::string& filename) {
    loadFromFile(filename);
}

std::string JsonConfig::readFile(const std::string& filename) const {
    std::ifstream file(filename);
    if (!file.is_open()) return "";

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string JsonConfig::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t\n\r\"");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r\"");
    return str.substr(first, last - first + 1);
}

std::map<std::string, std::string> JsonConfig::extractObject(const std::string& json) const {
    std::map<std::string, std::string> result;
    size_t pos = 0;

    while (pos < json.length()) {
        size_t keyStart = json.find('"', pos);
        if (keyStart == std::string::npos) break;

        size_t keyEnd = json.find('"', keyStart + 1);
        if (keyEnd == std::string::npos) break;

        std::string key = json.substr(keyStart + 1, keyEnd - keyStart - 1);

        size_t colonPos = json.find(':', keyEnd);
        if (colonPos == std::string::npos) break;

        size_t valueStart = colonPos + 1;
        while (valueStart < json.length() && std::isspace(json[valueStart])) valueStart++;

        if (valueStart >= json.length()) break;

        if (json[valueStart] == '"') {
            size_t valueEnd = json.find('"', valueStart + 1);
            if (valueEnd == std::string::npos) break;
            result[key] = json.substr(valueStart, valueEnd - valueStart + 1);
            pos = valueEnd + 1;
        }
        else if (json[valueStart] == '{' || json[valueStart] == '[') {
            char closingChar = (json[valueStart] == '{') ? '}' : ']';
            size_t valueEnd = json.find(closingChar, valueStart + 1);
            if (valueEnd == std::string::npos) break;
            result[key] = json.substr(valueStart, valueEnd - valueStart + 1);
            pos = valueEnd + 1;
        }
        else {
            size_t valueEnd = json.find_first_of(",}", valueStart);
            if (valueEnd == std::string::npos) valueEnd = json.length();
            result[key] = json.substr(valueStart, valueEnd - valueStart);
            pos = valueEnd;
        }

        pos = json.find_first_not_of(" \t\n\r,", pos);
    }

    return result;
}

std::string JsonConfig::extractValue(const std::string& json, const std::string& key) const {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";

    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if (colonPos == std::string::npos) return "";

    size_t valueStart = colonPos + 1;
    while (valueStart < json.length() && std::isspace(json[valueStart])) valueStart++;

    if (valueStart >= json.length()) return "";

    if (json[valueStart] == '"') {
        size_t valueEnd = json.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) return "";
        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    }
    else if (json[valueStart] == '{' || json[valueStart] == '[') {
        char closingChar = (json[valueStart] == '{') ? '}' : ']';
        size_t valueEnd = json.find(closingChar, valueStart + 1);
        if (valueEnd == std::string::npos) return "";
        return json.substr(valueStart, valueEnd - valueStart + 1);
    }
    else {
        size_t valueEnd = json.find_first_of(",}", valueStart);
        if (valueEnd == std::string::npos) valueEnd = json.length();
        std::string value = json.substr(valueStart, valueEnd - valueStart);
        return trim(value);
    }
}

std::vector<std::string> JsonConfig::extractArray(const std::string& json, const std::string& key) const {
    std::vector<std::string> result;

    std::string arrayStr = extractValue(json, key);
    if (arrayStr.empty() || arrayStr[0] != '[') return result;

    size_t pos = 1;
    while (pos < arrayStr.length()) {
        while (pos < arrayStr.length() && std::isspace(arrayStr[pos])) pos++;
        if (pos >= arrayStr.length() || arrayStr[pos] == ']') break;

        if (arrayStr[pos] == '{') {
            size_t objStart = pos;
            int braceCount = 1;
            pos++;

            while (pos < arrayStr.length() && braceCount > 0) {
                if (arrayStr[pos] == '{') braceCount++;
                else if (arrayStr[pos] == '}') braceCount--;
                pos++;
            }

            result.push_back(arrayStr.substr(objStart, pos - objStart));
        }
        else {
            size_t valueStart = pos;
            while (pos < arrayStr.length() && arrayStr[pos] != ',' && arrayStr[pos] != ']') {
                pos++;
            }
            result.push_back(arrayStr.substr(valueStart, pos - valueStart));
        }

        if (pos < arrayStr.length() && arrayStr[pos] == ',') pos++;
    }

    return result;
}

bool JsonConfig::loadFromFile(const std::string& filename) {
    std::string json = readFile(filename);
    if (json.empty()) {
        valid_ = false;
        return false;
    }

    auto root = extractObject(json);

    auto it = root.find("historySize");
    if (it != root.end()) historySize_ = std::stoi(trim(it->second));

    it = root.find("displayType");
    if (it != root.end()) displayType_ = trim(it->second);

    it = root.find("outputFile");
    if (it != root.end()) outputFile_ = trim(it->second);

    it = root.find("fileRotation");
    if (it != root.end()) fileRotation_ = (trim(it->second) == "true");

    it = root.find("maxFileSize");
    if (it != root.end()) maxFileSize_ = std::stoul(trim(it->second));

    filters_.clear();
    auto filterStrings = extractArray(json, "filters");

    for (const auto& filterStr : filterStrings) {
        auto filterObj = extractObject(filterStr);
        FilterConfig filter;

        auto fit = filterObj.find("type");
        if (fit != filterObj.end()) filter.type = trim(fit->second);

        fit = filterObj.find("enabled");
        if (fit != filterObj.end()) filter.enabled = (trim(fit->second) == "true");

        fit = filterObj.find("priority");
        if (fit != filterObj.end()) filter.priority = std::stoi(trim(fit->second));

        fit = filterObj.find("params");
        if (fit != filterObj.end()) {
            auto paramsObj = extractObject(fit->second);
            for (const auto& [pkey, pvalue] : paramsObj) {
                try {
                    filter.params[pkey] = std::stod(trim(pvalue));
                } catch (...) {}
            }
        }

        filters_.push_back(filter);
    }

    valid_ = true;
    return true;
}

bool JsonConfig::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << "{\n";
    file << "  \"historySize\": " << historySize_ << ",\n";
    file << "  \"displayType\": \"" << displayType_ << "\",\n";
    file << "  \"outputFile\": \"" << outputFile_ << "\",\n";
    file << "  \"fileRotation\": " << (fileRotation_ ? "true" : "false") << ",\n";
    file << "  \"maxFileSize\": " << maxFileSize_ << ",\n";
    file << "  \"filters\": [\n";

    for (size_t i = 0; i < filters_.size(); i++) {
        const auto& filter = filters_[i];
        file << "    {\n";
        file << "      \"type\": \"" << filter.type << "\",\n";
        file << "      \"enabled\": " << (filter.enabled ? "true" : "false") << ",\n";
        file << "      \"priority\": " << filter.priority << ",\n";
        file << "      \"params\": {\n";

        size_t j = 0;
        for (const auto& [key, value] : filter.params) {
            file << "        \"" << key << "\": " << value;
            if (j < filter.params.size() - 1) file << ",";
            file << "\n";
            j++;
        }

        file << "      }\n";
        file << "    }";
        if (i < filters_.size() - 1) file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";

    return true;
}
