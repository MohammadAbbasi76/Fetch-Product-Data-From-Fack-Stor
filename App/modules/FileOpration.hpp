#pragma once
#include "DataProcessing.hpp"
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

class FileOpration {
public:
  explicit FileOpration(const std::string &baseDir = "data");
  ~FileOpration() = default;
  int ModifyCSV(const std::vector<Product> &products);

private:
  static bool DirectoryExists(const std::string &path);
  static bool MakeDirIfNeeded(const std::string &path);
  static long FileSizeIfExists(const std::string &path);

  std::string GetBaseDir() const { return BaseDir; }
  std::string GetFilePath() const { return FilePath; }
  std::string CheckAndCreateDir();
  static std::string EscapeCSV(const std::string &s);

  std::string BaseDir;
  std::string FilePath;
  const std::string filename = "products.csv";
};
