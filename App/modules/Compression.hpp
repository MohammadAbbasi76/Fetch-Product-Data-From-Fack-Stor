#pragma once
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <zip.h>

class Compression {
public:
  Compression(const std::string &sourceFolder = "data",
              const std::string &zipFile = "data.zip");
  ~Compression() = default;
  int CompressFolder();

private:
  void ListFilesRecursive(const std::string &base, const std::string &path,
                            std::vector<std::string> &files);
  bool AddFileToZip(zip_t *archive, const std::string &base,
                       const std::string &relative_path);
  std::string SourceFolder;
  std::string ZipFile;
};
