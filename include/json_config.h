#pragma once

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

struct FilterConfig {
    std::string type;
    bool enabled = true;
    int priority = 0;
    std::map<std::string, double> params;
};

class JsonConfig {
public:
    JsonConfig();
    explicit JsonConfig(const std::string& filename);
    
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;
    
    int getHistorySize() const { return historySize_; }
    const std::string& getDisplayType() const { return displayType_; }
    const std::string& getOutputFile() const { return outputFile_; }
    bool isFileRotation() const { return fileRotation_; }
    size_t getMaxFileSize() const { return maxFileSize_; }
    const std::vector<FilterConfig>& getFilters() const { return filters_; }
    
    void setHistorySize(int size) { historySize_ = size; }
    void setDisplayType(const std::string& type) { displayType_ = type; }
    void setOutputFile(const std::string& file) { outputFile_ = file; }
    void setFileRotation(bool rotate) { fileRotation_ = rotate; }
    void setMaxFileSize(size_t size) { maxFileSize_ = size; }
    void addFilter(const FilterConfig& filter) { filters_.push_back(filter); }
    void clearFilters() { filters_.clear(); }
    
    bool isValid() const { return valid_; }

private:
    std::string readFile(const std::string& filename) const;
    std::string trim(const std::string& str) const;
    std::string extractValue(const std::string& json, const std::string& key) const;
    std::vector<std::string> extractArray(const std::string& json, const std::string& key) const;
    std::map<std::string, std::string> extractObject(const std::string& json) const;
    
    int historySize_ = 10;
    std::string displayType_ = "console";
    std::string outputFile_;
    bool fileRotation_ = false;
    size_t maxFileSize_ = 1024 * 1024;
    std::vector<FilterConfig> filters_;
    bool valid_ = true;
};