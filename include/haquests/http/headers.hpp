#pragma once

#include <string>
#include <map>
#include <vector>

namespace haquests {
namespace http {

class Headers {
public:
    Headers();
    
    // Add header (allows duplicates)
    void add(const std::string& key, const std::string& value);
    
    // Set header (replaces existing)
    void set(const std::string& key, const std::string& value);
    
    // Get header
    std::string get(const std::string& key) const;
    std::vector<std::string> getAll(const std::string& key) const;
    
    // Check if exists
    bool has(const std::string& key) const;
    
    // Remove header
    void remove(const std::string& key);
    
    // Parse from string
    bool parse(const std::string& header_section);
    
    // Build string
    std::string build() const;
    
    // Get all headers
    const std::multimap<std::string, std::string>& getAll() const;

private:
    std::multimap<std::string, std::string> headers_;
    
    std::string normalizeKey(const std::string& key) const;
};

} // namespace http
} // namespace haquests
