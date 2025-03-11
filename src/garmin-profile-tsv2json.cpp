#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <json/json.h>

struct Field {
    int number;
    std::string name;
    std::string type;
    bool is_array = false;
    double scale = 1.0;
    double offset = 0.0;
    std::string units;
};

// Function to trim whitespace from a string_view
std::string trim(std::string_view str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == std::string_view::npos) ? "" : std::string(str.substr(start, end - start + 1));
}

// Function to split a TSV line while preserving empty columns
std::vector<std::string> split_tsv_line(std::string_view line) {
    std::vector<std::string> result;
    size_t start = 0, end;
    
    do {
        end = line.find('\t', start);
        result.emplace_back(trim(line.substr(start, (end == std::string_view::npos ? end : end - start))));
        start = end + 1;
    } while (end != std::string_view::npos);

    return result;
}

// Function to parse a TSV file into a JSON structure
Json::Value parse_tsv_to_json(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return {};
    }

    Json::Value root;
    std::string current_message_type;
    Json::Value* current_fields = nullptr;
    
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> fields = split_tsv_line(line);

        // Ignore empty lines or invalid lines
        if (fields.size() < 2 || (fields[0].empty() && fields[1].empty())) continue;

        if (!fields[0].empty()) {
            // New message type
            current_message_type = fields[0];
            current_fields = &root[current_message_type]["fields"];
        }

        if (current_fields && !fields[1].empty()) {
            Field field;
            field.number = std::stoi(fields[1]);
            field.name = fields[2];
            field.type = fields[3];

            if (fields.size() > 4) {
                field.is_array = (fields[4] == "[N]");
            }
            if (fields.size() > 6 && !fields[6].empty()) {
                field.scale = std::stod(fields[6]);
            }
            if (fields.size() > 7 && !fields[7].empty()) {
                field.offset = std::stod(fields[7]);
            }
            if (fields.size() > 8) {
                field.units = fields[8];
            }

            // Convert field to JSON object
            Json::Value json_field;
            json_field["number"] = field.number;
            json_field["name"] = field.name;
            json_field["type"] = field.type;
            if (field.is_array) json_field["is_array"] = true;
            if (!field.units.empty()) json_field["units"] = field.units;
            if (field.scale != 1.0) json_field["scale"] = field.scale;
            if (field.offset != 0.0) json_field["offset"] = field.offset;

            (*current_fields).append(json_field);
        }
    }

    return root;
}

// Function to save JSON to a file
void save_json_to_file(const Json::Value& json, const std::string& filename) {
    std::ofstream file(filename, std::ios::out);
    if (!file) {
        std::cerr << "Error writing to file: " << filename << std::endl;
        return;
    }

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "  "; // Pretty-print with indentation
    file << Json::writeString(writer, json);
    std::cout << "JSON saved to " << filename << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return 1;
    }

    const std::string input_file = argv[1];
    const std::string output_file = argv[2];

    Json::Value json = parse_tsv_to_json(input_file);
    save_json_to_file(json, output_file);

    return 0;
}
