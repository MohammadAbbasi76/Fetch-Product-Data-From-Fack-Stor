// #include "modules/ReceiveData.hpp"
// #include "modules/DataProcessing.hpp"
// #include <iostream>
// #include <string>
// #include <vector>
// #include "modules/FileOpration.cpp"
// int main() {
//     // Example with live data from API
//     // std::string url = "https://fakestoreapi.com/products";
//     // ReceiveData receiver(url);
    
//     // try {
//     //     // Fetch the data from the API
//     //     std::string jsonData = receiver.SendRquestAndHandleIt();
        
//     //     // Process the received JSON data
//     //     DataProcessing processor(jsonData);
//     //     std::vector<Product> products = processor.parseJsonData();
        
//     //     // Print the results
//     //     std::cout << "\nSuccessfully processed " << processor.getProductCount() << " products:\n\n";
        
//     //     // for (const auto& product : products) {
//     //     //     std::cout << "ID: " << product.id << "\n"
//     //     //              << "Title: " << product.title << "\n"
//     //     //              << "Price: $" << product.price << "\n"
//     //     //              << "Category: " << product.category << "\n"
//     //     //              << "------------------------\n";
//     //     // }
        
//     // } catch (const std::exception& e) {
//     //     std::cerr << "Error: " << e.what() << std::endl;
//     //     return 1;
//     // }
//     test();
//     return 0;
// }

// json_to_csv_no_filesystem.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cerrno>
#include <cstring>
#include <sys/stat.h> // mkdir
#include <unistd.h>   // getcwd
#include <limits.h>   // PATH_MAX
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static std::string escape_csv(const std::string &s) {
    bool need_quotes = s.find_first_of(",\"\n\r") != std::string::npos;
    if (!need_quotes) return s;
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += "\"\"";
        else out += c;
    }
    out += "\"";
    return out;
}

bool dir_exists(const std::string &path) {
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

bool make_dir_if_needed(const std::string &path) {
    if (dir_exists(path)) return true;
    // mode 0755
    if (mkdir(path.c_str(), 0755) == 0) return true;
    if (errno == EEXIST) return dir_exists(path); // race: someone else created it
    return false;
}

long file_size_if_exists(const std::string &path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in.is_open()) return -1;
    return static_cast<long>(in.tellg());
}

int main() {
    const std::string dir = "data";       // output folder
    const std::string filename = "products.csv";
    const std::string filepath = dir + "/" + filename;

    // --- ensure directory exists ---
    if (!make_dir_if_needed(dir)) {
        std::cerr << "Failed to create directory '" << dir << "': " << std::strerror(errno) << "\n";
        return 1;
    }

    // --- sample JSON items ---
    std::vector<json> products = {
        { {"ID", 1}, {"Title", "Mens Casual Premium Slim Fit T-Shirts"}, {"Price", 22.3}, {"Category", "men's clothing"} },
        { {"ID", 2}, {"Title", "Women's Casual Top"}, {"Price", 18.5}, {"Category", "women's clothing"} },
        { {"ID", 3}, {"Title", "Smartphone 128GB"}, {"Price", 499.99}, {"Category", "electronics"} }
    };

    // --- decide whether header is needed (file exists and non-empty -> no header) ---
    bool write_header = true;
    long sz = file_size_if_exists(filepath);
    if (sz > 0) write_header = false;

    // --- open csv in append mode ---
    std::ofstream csv(filepath, std::ios::app);
    if (!csv.is_open()) {
        std::cerr << "Failed to open '" << filepath << "' for writing.\n";
        return 1;
    }

    if (write_header) {
        csv << "ID,Title,Price,Category\n";
    }

    // --- write rows ---
    for (const auto &item : products) {
        int id = 0;
        if (item.contains("ID")) {
            try { id = item.at("ID").get<int>(); } catch (...) { id = 0; }
        }
        std::string title = "";
        if (item.contains("Title")) {
            try { title = item.at("Title").get<std::string>(); } catch (...) { title = ""; }
        }
        double price = 0.0;
        if (item.contains("Price")) {
            try { price = item.at("Price").get<double>(); } catch (...) { price = 0.0; }
        }
        std::string category = "";
        if (item.contains("Category")) {
            try { category = item.at("Category").get<std::string>(); } catch (...) { category = ""; }
        }

        csv << id << ","
            << escape_csv(title) << ","
            << price << ","
            << escape_csv(category) << "\n";
    }

    csv.close();

    // --- print absolute path (getcwd + relative path) ---
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << "Wrote " << products.size() << " items to '" << filepath << "'.\n";
        std::cout << "Full path: " << std::string(cwd) + "/" + filepath << "\n";
    } else {
        std::cout << "Wrote " << products.size() << " items to '" << filepath << "'.\n";
    }

    return 0;
}

