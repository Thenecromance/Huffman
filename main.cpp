//
// Created by Thenecromance on 2023/9/8.
//

#include <fstream>
#include <iostream>

#include "Huffman.hpp"
std::string orig_file = "./Test.Http.exe";
std::string write_file = "./test.bin";
int EncodeTest() {
  std::string buffer;

  {
    std::fstream fs(orig_file, std::ios::in | std::ios::binary);
    if (!fs.is_open()) {
      std::cout << orig_file << "Open file failed" << std::endl;
      return -1;
    }

    fs.seekg(0, std::ios::end);
    buffer.resize(fs.tellg());
    fs.seekg(0, std::ios::beg);
    fs.read(&buffer[0], buffer.size());
    fs.close();
  }
  Huffman huffman;
  auto encoded = huffman.Encode(buffer);
  //  huffman.PrintHuffmanTree();
  huffman.ShowCompressResult();
  {
    std::fstream fs(write_file, std::ios::out | std::ios::binary);
    fs.write(encoded.c_str(), encoded.length());
    fs.close();
  }
  return 0;
}

auto DecodeTest() {
  std::string buffer;
  std::string data;
  // decode data
  {
    Huffman decodeTest;
    std::fstream fs(write_file, std::ios::in | std::ios::binary);
    if (!fs.is_open()) {
      std::cout << write_file << "Open file failed" << std::endl;

      return -1;
    }
    fs.seekg(0, std::ios::end);
    buffer.resize(fs.tellg());
    fs.seekg(0, std::ios::beg);
    fs.read(&buffer[0], buffer.size());
    fs.close();
    std::cout << "Decoded data:\n";
    // std::cout << ;
    data = decodeTest.Decode(buffer);
  }
  // Write the original data to local file
  {
    std::fstream re_write("./Decompressed.exe",
                          std::ios::out | std::ios::binary);
    re_write.write(data.c_str(), data.size());
    re_write.close();
  }
  return 0;
}
auto main() -> int {
  try {
    EncodeTest();
    DecodeTest();
    return 0;

  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    exit(1337);
  }
}
