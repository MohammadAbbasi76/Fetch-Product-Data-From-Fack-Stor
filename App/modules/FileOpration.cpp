
#include "FileOpration.hpp"

// Constructor
FileOpration::FileOpration(const std::string& baseDir) 
    : BaseDir(baseDir) {
    FilePath = BaseDir + "/" + filename;
}

// Static helper method for CSV escaping
std::string FileOpration::EscapeCSV(const std::string &s) {
    bool need_quotes = s.find_first_of(",\"\n\r") != std::string::npos;
    if (!need_quotes)
        return s;
    
    std::string out = "\"";
    for (char c : s) {
        if (c == '"')
            out += "\"\"";
        else
            out += c;
    }
    out += "\"";
    return out;
}

// Check if directory exists
bool FileOpration::DirectoryExists(const std::string &path) {
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

// Create directory if it doesn't exist
bool FileOpration::MakeDirIfNeeded(const std::string &path) {
    if (DirectoryExists(path))
        return true;
    
    if (mkdir(path.c_str(), 0755) == 0)
        return true;
    
    if (errno == EEXIST)
        return DirectoryExists(path); 
    
    return false;
}

// Get file size if file exists
long FileOpration::FileSizeIfExists(const std::string &path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in.is_open())
        return -1;
    return static_cast<long>(in.tellg());
}

// Helper method to check and create directory
std::string FileOpration::CheckAndCreateDir() {
    if (!MakeDirIfNeeded(BaseDir)) {
        std::cerr << "Failed to create directory '" << BaseDir
                  << "': " << std::strerror(errno) << "\n";
        return "failed";
    }
    return FilePath;
}

// Main method to modify CSV
int FileOpration::ModifyCSV(const std::vector<Product> &products) {
    // Check and create directory if needed
    std::string checkedPath = CheckAndCreateDir();
    if (checkedPath == "failed") {
        return 1;
    }
    
    bool write_header = true;
    long sz = FileSizeIfExists(FilePath);
    if (sz > 0)
        write_header = false;

    std::ofstream csv(FilePath, std::ios::app);
    if (!csv.is_open()) {
        std::cerr << "Failed to open '" << FilePath << "' for writing.\n";
        return 1;
    }

    if (write_header) {
        csv << "ID,Title,Price,Category\n";
    }

    for (const auto &item : products) {
        csv << item.id << ","
            << EscapeCSV(item.title) << ","
            << item.price << ","
            << EscapeCSV(item.category) << "\n";
    }
    
    csv.close();
    return 0;
}
