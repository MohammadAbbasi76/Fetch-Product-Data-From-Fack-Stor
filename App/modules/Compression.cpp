
#include "Compression.hpp"

// Constructor implementation
Compression::Compression(const std::string& sourceFolder, const std::string& zipFile)
    : SourceFolder(sourceFolder)
    , ZipFile(zipFile) {
}

void Compression::ListFilesRecursive(const std::string &base, const std::string &path, std::vector<std::string> &files) {
    std::string full = base + "/" + path;
    DIR *dir = opendir(full.c_str());
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        std::string relative_path = path.empty() ? name : path + "/" + name;
        std::string full_path = base + "/" + relative_path;

        struct stat st;
        if (stat(full_path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                ListFilesRecursive(base, relative_path, files);
            } else if (S_ISREG(st.st_mode)) {
                files.push_back(relative_path);
            }
        }
    }
    closedir(dir);
}

bool Compression::AddFileToZip(zip_t *archive, const std::string &base, const std::string &relative_path) {
    std::string full_path = base + "/" + relative_path;
    std::ifstream file(full_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << full_path << "\n";
        return false;
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    zip_source_t *source = zip_source_buffer(archive, buffer.data(), buffer.size(), 0);
    if (!source) {
        std::cerr << "Error creating zip source for: " << full_path << "\n";
        return false;
    }

    if (zip_file_add(archive, relative_path.c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        std::cerr << "Error adding to zip: " << full_path << " (" << zip_strerror(archive) << ")\n";
        zip_source_free(source);
        return false;
    }

    return true;
}

int Compression::CompressFolder() {

    struct stat st;
    if (stat(SourceFolder.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
        std::cerr << "Folder '" << SourceFolder << "' does not exist.\n";
        return 1;
    }

    int errorp;
    zip_t *archive = zip_open(ZipFile.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorp);
    if (!archive) {
        std::cerr << "Failed to create ZIP file: " << ZipFile << "\n";
        return 1;
    }

    std::vector<std::string> files;
    ListFilesRecursive(SourceFolder, "", files);

    bool success = true;
    for (const auto &rel_path : files) {
        if (!AddFileToZip(archive, SourceFolder, rel_path)) {
            std::cerr << "Failed to add " << rel_path << " to zip.\n";
            success = false;
        }
    }

    if (zip_close(archive) != 0) {
        std::cerr << "Error closing zip: " << zip_strerror(archive) << "\n";
        return 1;
    }

    if (success) {
        std::cout << "Folder '" << SourceFolder << "' successfully compressed into '" << ZipFile << "'.\n";
        return 0;
    }
    return 1;
}
