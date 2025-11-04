#include"FileOpration.hpp"
// json_to_csv.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

static std::string escape_csv(const std::string &s) {
    // If string contains quote, comma, or newline, wrap in quotes and double any quotes inside.
    bool need_quotes = s.find_first_of(",\"\n\r") != std::string::npos;
    if (!need_quotes) return s;
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += "\"\"";  // double the quote
        else out += c;
    }
    out += "\"";
    return out;
}

int test() {
    // --- Configure output file path ---
    const std::string dir = "data";                     // change to desired folder
    const std::string filename = "products.csv";
    const fs::path folder_path = fs::path(dir);
    const fs::path file_path = folder_path / filename;

    try {
        // create folder if it doesn't exist
        if (!fs::exists(folder_path)) {
            fs::create_directories(folder_path);
        }
    } catch (const std::exception &e) {
        std::cerr << "Failed to create directory '" << folder_path.string() << "': " << e.what() << "\n";
        return 1;
    }

    // --- Sample JSON items to write (you can replace these or load from a file) ---
    std::vector<json> products = {
        { {"ID", 1}, {"Title", "Mens Casual Premium Slim Fit T-Shirts"}, {"Price", 22.3}, {"Category", "men's clothing"} },
        { {"ID", 2}, {"Title", "Women's Casual Top"}, {"Price", 18.5}, {"Category", "women's clothing"} },
        { {"ID", 3}, {"Title", "Smartphone 128GB"}, {"Price", 499.99}, {"Category", "electronics"} }
    };

    // --- Open CSV for append, but write header if file is new/empty ---
    bool write_header = true;
    if (fs::exists(file_path)) {
        // check size; if > 0 assume header exists
        try {
            if (fs::file_size(file_path) > 0) write_header = false;
        } catch (...) {
            // if file_size fails, keep header true (safe fallback)
        }
    }

    std::ofstream csv(file_path, std::ios::app); // append mode
    if (!csv.is_open()) {
        std::cerr << "Failed to open '" << file_path.string() << "' for writing.\n";
        return 1;
    }

    if (write_header) {
        csv << "ID,Title,Price,Category\n";
    }

    // --- Write JSON items as CSV rows ---
    for (const auto &item : products) {
        // Safely extract values: if missing fields use empty/defaults
        int id = 0;
        if (item.contains("ID")) {
            try { id = item.at("ID").get<int>(); } catch (...) { id = 0; }
        }

        std::string title = "";
        if (item.contains("Title")) {
            try { title = item.at("Title").get<std::string>(); } catch (...) { title = ""; }
        }

        // Price as number (keeps decimals), but write as text to avoid locale issues
        double price = 0.0;
        if (item.contains("Price")) {
            try { price = item.at("Price").get<double>(); } catch (...) { price = 0.0; }
        }

        std::string category = "";
        if (item.contains("Category")) {
            try { category = item.at("Category").get<std::string>(); } catch (...) { category = ""; }
        }

        // Escape text fields and write
        csv << id << ","
            << escape_csv(title) << ","
            // Write price with default formatting (you can change precision if needed)
            << price << ","
            << escape_csv(category) << "\n";
    }

    csv.close();
    std::cout << "Wrote " << products.size() << " items to '" << file_path.string() << "'.\n";
    std::cout << "Full path: " << fs::absolute(file_path) << "\n";
    return 0;
}
