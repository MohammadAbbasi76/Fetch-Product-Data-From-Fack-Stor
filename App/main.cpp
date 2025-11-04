#include "modules/Compression.hpp"
#include "modules/DataProcessing.hpp"
#include "modules/FileOpration.hpp"
#include "modules/ReceiveData.hpp"
#include <iostream>
#include <string>
#include <vector>

int main() {
  std::string url = "https://fakestoreapi.com/products";
  ReceiveData receiver(url);

  try {
    
    // Fetch the data from the API
    std::string jsonData = receiver.SendRquestAndHandleIt();

    // Process the received JSON data
    DataProcessing processor(jsonData);
    std::vector<Product> products = processor.parseJsonData();

    // Modify the CSV file with the processed data
    FileOpration fileOp("data");
    int result = fileOp.ModifyCSV(products);
    if (result != 0) {
      std::cerr << "Failed to modify CSV file." << std::endl;
      return 1;
    } else {
      std::cout << "CSV file modified successfully." << std::endl;
    }

    // Compress the data folder into a ZIP file
    Compression compressor("data", "data.zip");
    int compressResult = compressor.CompressFolder();
    if (compressResult != 0) {
      std::cerr << "Failed to compress folder." << std::endl;
      return 1;
    }
    return 0;



  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
