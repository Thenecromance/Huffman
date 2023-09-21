#pragma once
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>

static constexpr auto kChildSize = sizeof(short);
namespace detail {

/// @brief Huffman Tree Node Implements
struct Node {
  /// @brief for build tree
  short self_idx{-1};
  ///@brief when write to the file , freq is useless, just for compress so after
  /// load from file, freq is -1
  int freq{-1};
  /// @brief if is leaf node, left will be -1, or it will point to the left
  /// child node
  short left{-1};
  /// @brief if is leaf node, right will be -1, or it will point to the right
  /// child node
  short right{-1};
  /// @brief save the char data for leaf node, otherwise it will be 0
  char data{0};
  /// @brief check node has right child
  /// @return if has right child return true
  [[nodiscard]] bool RightNull() const { return right == -1; }
  /// @brief check node has left child
  /// @return if has left child return true
  [[nodiscard]] bool LeftNull() const { return left == -1; }
  /// @brief check node is leaf
  /// @return if is leaf return true
  [[nodiscard]] bool IsLeaf() const { return RightNull() && LeftNull(); }
};

struct Compare {
  bool operator()(const Node& lhs, const Node& rhs) const {
    return lhs.freq > rhs.freq;
  }
};
/// @brief for binary file compress and decompress
struct Info {
  /// @brief huffman's tree size
  unsigned tree_size;
  /// @brief using 8 bytes to save leaf count,the leaf nodes nodes data in file
  /// will be only use 1byte(16bytes to 1 byte! only saved Node::data)and other
  /// nodes will be use 4 bytes(only saved Node::left and Node::right), that's
  /// too cheap
  unsigned leaf_count;
  /// @brief because of I use char to save all the data, in some times, the end
  /// of code path will be less than 8 bits, so I use 0 to fill the rest of the
  /// bits
  // of course there is another way to solve this problem, such as use packed
  // bits therefore, I can simply use 1 byte to save the data, but what
  // ever,it's already has 4 padding, this is enough
  unsigned bits;
  /// @brief All tree node will be saved here and DO NOT directly operate this
  /// or change it sequence otherwise it will cause the tree error
  /// @note  Nodes' relations are based on the sequence of the vector
  std::vector<Node> tree;
};
/// @brief all compress result info
struct CompressResult {
  unsigned leaf_count;
  unsigned node_count;
  unsigned original_size;
  unsigned compressed_size;
  float compression_ratio;
};
};  // namespace detail

using FrequencyMap = std::map<char, int>;
/// @brief Huffman Tree Builder
class TreeBuilder {
 private:
  detail::Info& m_info;
  FrequencyMap& m_frequencies;

 public:
  /// @brief Huffman tree builder constructor
  /// @param info Huffman info from Huffman class
  /// @param frequencies char's frequency from Huffman class
  TreeBuilder(detail::Info& info, FrequencyMap& frequencies);
  /// @brief build tree by frequencies ONLY when Compress the data (decompress
  /// info does not contain frequencies)
  void BuildTree();
  /// @brief build tree by loaded from the file
  /// @param ss when after parsing the basic info(which it will be parsed by
  /// Huffman), the rest of the data will be stored in the stringstream
  void BuildTree(std::stringstream& ss);
  /// @brief for save huffman tree info
  /// @return result of the tree info
  std::string to_str();
};

/// @brief Huffman Compressor and Decompressor
class Huffman {
  /// @brief for compress and decompress
  // by the way all the Node data will be also stored in the file
  detail::Info m_info;
  /// @brief each char's frequency
  FrequencyMap m_frequencies{};

#ifdef NO_TREE_PRINT
  /// @brief for writing the Tree to the file(if don't need to print the Tree,
  /// this is useless)
  std::unique_ptr<std::fstream> p_stream{nullptr};
#endif  // NO_TREE_PRINT

  /// @brief for compress result which it will contain the original size and
  /// compressed size only needed when you want to know the compression ratio or
  /// other info
  std::unique_ptr<detail::CompressResult> p_result;
  // using special template about std::vector<bool> to store huffman codes
  using CodePath = std::vector<bool>;
  /// @brief after generate the huffman tree, all code path will be saved in
  /// this map
  std::map<char, CodePath> m_codes{};
  /// @brief for building huffman tree
  std::unique_ptr<TreeBuilder> p_builder;

 public:
  Huffman();
  ~Huffman();

 public:
  /// @brief Encode the data
  /// @param buffer whole file buffer
  /// @return Huffman tree info and Encoded data
  std::string Encode(const std::string& buffer);
  /// @brief Decode the data from the local file or stream
  /// @param buffer the stream for the encoded data
  /// @return The decoded data(without Huffman tree info)you can directly use it
  std::string Decode(const std::string& buffer);

#ifdef NO_TREE_PRINT
  /// @brief Print the Huffman tree to the file, the reason why I write to file
  /// is in my terminal if the tree goes to Console, it looks very ugly
  void PrintHuffmanTree();
#endif  // NO_TREE_PRINT

  /// @brief Print the each char's code path to the Console
  void ShowCode();
  /// @brief Print the compress result info to the Console
  void ShowCompressResult();

 private:
  /// @brief Get each chars' frequency in data
  /// @param buffer the buffer data
  void GetFrequencies(const std::string& buffer);
  /// @brief for building huffman tree code path
  /// @param root the root node(or you can easy to use m_info.tree.back())
  /// @param code the container for saving the code path
  void GenerateCodes(detail::Node& root, const CodePath& code);
  /// @brief Merge the Huffman info and the compressed data
  /// @param result Encoded data
  /// @return the final stream for writing to the file
  std::string BuildBuffer(std::vector<bool>& result);
  /// @brief due to generate the huffman tree, m_info.tree.back() will be the
  /// root of the tree
  /// @return root node
  detail::Node& GetRoot() { return m_info.tree.back(); }
  /// @brief Print the compress result info
  void PrintCompressInfo(const std::string& buffer,
                         const std::string& encoded_str);

#ifdef NO_TREE_PRINT
 private:
  /// @brief Print the Tree into the files Implements
  /// @param data
  /// @param n
  /// @param left
  /// @param indent
  void output_impl(std::vector<detail::Node>& data, detail::Node& n, bool left,
                   std::string const& indent);
  /// @brief print the tree to the file
  /// @param data m_info.tree
  /// @param root m_info.tree.back()
  void PrintTreeToFile(std::vector<detail::Node>& data, detail::Node& root);
#endif  // NO_TREE_PRINT
};