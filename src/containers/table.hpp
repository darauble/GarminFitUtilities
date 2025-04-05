#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace darauble::containers {

class Table {
protected:
    std::vector<std::string> header;
    std::unordered_map<std::string, size_t> tableWidths;
    std::vector<std::unordered_map<std::string, std::string>> data;
    
public:
    Table(std::vector<std::string> _header) :
        header {_header}
    {
        for (const auto& column : _header) {
            tableWidths[column] = utf8Length(column);
        }
    }

    void addRow(const std::unordered_map<std::string, std::string>& row) {
        data.push_back(row);

        for (const auto& [key, value] : row) {
            if (tableWidths.contains(key)) {
                tableWidths[key] = std::max(tableWidths.at(key), utf8Length(value));
            }
        }
    }
    
    void sortByColumn(const std::string& column) {
        std::sort(data.begin(), data.end(), [&column](const auto& a, const auto& b) {
            return a.at(column) < b.at(column);
        });
    }

    const std::vector<std::string>& getHeader() {
        return header;
    }

    const std::unordered_map<std::string, size_t>& getTableWidths() {
        return tableWidths;
    }

    const std::vector<std::unordered_map<std::string, std::string>>& getData() {
        return data;
    }

    static size_t utf8Length(const std::string& str) {
        size_t length = 0;
     
        for (char value : str) {
            if ((value & 0xc0) != 0x80) {
                ++length;
            }
        }

        return length;
    }

    friend std::ostream& operator <<(std::ostream& os, const Table& table) {
        std::stringstream separator, header;
    
        separator << "+";
        header << "|";
    
        for (const auto& column : table.header) {
            separator << "-" << std::setw(table.tableWidths.at(column)) << std::setfill('-') << "-" << "-+";
            header << " " << std::setw(table.tableWidths.at(column)) << std::setfill(' ') << column << " |";
        }
    
        header << std::endl;
        separator << std::endl;
    
        os << separator.str() << header.str() << separator.str();
    
        for (const auto& row : table.data) {
            os << "|";
    
            for (const auto& column : table.header) {
                os << " " << std::setw(table.tableWidths.at(column) + row.at(column).length() - utf8Length(row.at(column))) << std::setfill(' ') << row.at(column) << " |";
            }
            
            os << std::endl;
        }
    
        os << separator.str();
    
        return os;
    }
};

} // namespace darauble::containers
