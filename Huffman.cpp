
#include "Huffman.hpp"

using namespace detail;

template <typename Type = int>
std::vector<Type> MergeBits(std::vector<bool>& bits) {
  std::vector<Type> result;
  Type current_val = 0;
  int bit_count = 0;
  for (bool bit : bits) {
    current_val |= (bit << bit_count);
    bit_count++;
    if (bit_count == 8 * sizeof(Type)) {
      result.push_back(current_val);
      current_val = 0;
      bit_count = 0;
    }
  }
  if (bit_count > 0) {
    result.push_back(current_val);
  }
  return result;
}

std::vector<bool> ExtractBuffer(const std::string& buffer) {
  std::vector<bool> result;
  for (auto& c : buffer) {
    for (int i = 0; i < 8; i++) {
      result.push_back(c & (1 << i));
    }
  }
  return result;
}

std::string Huffman::Encode(const std::string& buffer) {
  p_result = std::make_unique<detail::CompressResult>();
  GetFrequencies(buffer);
  m_info.leaf_count = m_frequencies.size();
  // BuildTree();
  p_builder->BuildTree();
  GenerateCodes(GetRoot(), CodePath());

  // Encode the buffer to the binary type which it will be saved by
  // std::vector<bool>
  std::vector<bool> result;
  for (char i : buffer) {
    auto& code_path = m_codes.at(i);
    result.insert(result.end(), code_path.begin(), code_path.end());
  }

  // when save to the file, it will cast to char to save the data. though in
  // some case, the encoded val may be need to packed 0 to it. After that when
  // need to Decode it self again, I need to know how much bits I need to bypass
  m_info.bits = result.size();

  auto encoded_str = BuildBuffer(result);

  PrintCompressInfo(buffer, encoded_str);
  return std::move(encoded_str);
}

std::string Huffman::Decode(const std::string& buffer) {
  std::stringstream ss(buffer);

  ss.read(reinterpret_cast<char*>(&m_info.tree_size), sizeof(m_info.tree_size));
  ss.read(reinterpret_cast<char*>(&m_info.leaf_count),
          sizeof(m_info.leaf_count));
  ss.read(reinterpret_cast<char*>(&m_info.bits), sizeof(m_info.bits));
  // read tree
  p_builder->BuildTree(ss);

  // read raw buffer bits by bits
  std::vector<bool> result;
  char c;
  while (ss.read(&c, sizeof(c))) {
    for (int i = 0; i < 8; i++) {
      result.push_back(c & (1 << i));
    }
  }
  // remove padding bits
  if (result.size() > m_info.bits) {
    std::cout << "erased invaild bits count:" << result.size() - m_info.bits
              << std::endl;
    result.erase(result.begin() + m_info.bits, result.end());
  }

  // the local file already include each Node's relation ship and the location's
  // in the tree.
  // so just directly use the tree to decode the data
  GenerateCodes(m_info.tree.back(), CodePath());

  // CodePath to string
  std::string data;
  auto root = &m_info.tree.back();
  for (auto&& bit : result) {
    root = &m_info.tree[bit ? root->right : root->left];
    if (root->IsLeaf()) {
      data += root->data;
      root = &m_info.tree.back();
    }
  }

  return std::move(data);
}

#ifdef NO_TREE_PRINT
void Huffman::PrintHuffmanTree() {
  if (p_stream == nullptr)
    p_stream = std::make_unique<std::fstream>("tree.txt", std::ios::out);
  PrintTreeToFile(m_info.tree, GetRoot());
}
#endif  // NO_TREE_PRINT

void Huffman::GetFrequencies(const std::string& buffer) {
  for (auto& c : buffer) m_frequencies[c]++;
}

void Huffman::PrintCompressInfo(
    const std::string& buffer,
    const std::string& encoded_str)  // save the compress result
{
  p_result->leaf_count = m_frequencies.size();
  p_result->node_count = m_info.tree.size();
  p_result->original_size = buffer.size();
  p_result->compressed_size = encoded_str.size();
  p_result->compression_ratio =
      static_cast<float>(p_result->compressed_size) / p_result->original_size;
}

#ifdef NO_TREE_PRINT
void Huffman::output_impl(std::vector<Node>& data, Node& n, bool left,
                          std::string const& indent) {
  if (n.right != -1) {
    output_impl(data, data[n.right], false,
                indent + (left ? "|     " : "      "));
  }

  *p_stream << indent;
  *p_stream << (left != -1 ? '\\' : '/');
  *p_stream << "-----";
  *p_stream << n.data << std::endl;
  if (n.left != -1) {
    output_impl(data, data[n.left], true,
                indent + (left ? "      " : "|     "));
  }
}
void Huffman::PrintTreeToFile(std::vector<detail::Node>& data,
                              detail::Node& root) {
  if (!root.RightNull()) {
    output_impl(data, data[root.right], false, "");
  }
  std::cout << root.data << std::endl;
  if (!root.LeftNull()) {
    output_impl(data, data[root.left], true, "");
  }
}
#endif

void Huffman::ShowCode() {
  std::cout << "leaf count:" << m_frequencies.size() << std::endl;
  std::cout << "=============Code Map=============\n";
  for (auto& pair : m_codes) {
    std::cout << pair.first << " | ";
    for (bool bit : pair.second) std::cout << bit;
    std::cout << std::endl;
  }
  std::cout << "==============END==============\n";
}

void Huffman::GenerateCodes(detail::Node& root, const CodePath& code) {
  if (!root.LeftNull()) {
    auto left = code;
    left.push_back(false);
    GenerateCodes(m_info.tree[root.left], left);
  }
  if (!root.RightNull()) {
    CodePath right = code;
    right.push_back(true);
    GenerateCodes(m_info.tree[root.right], right);
  }
  if (root.IsLeaf()) {
    m_codes[root.data] = code;
  }
}

std::string Huffman::BuildBuffer(std::vector<bool>& result) {
  std::stringstream ss;
  ss.write(reinterpret_cast<char*>(&m_info.tree_size),
           sizeof(m_info.tree_size));
  ss.write(reinterpret_cast<char*>(&m_info.leaf_count),
           sizeof(m_info.leaf_count));
  ss.write(reinterpret_cast<char*>(&m_info.bits), sizeof(m_info.bits));
  ss.write(p_builder->to_str().c_str(), p_builder->to_str().length());
  auto data = MergeBits<char>(result);
  ss.write(data.data(), data.size());
  return ss.str();
}
void Huffman::ShowCompressResult() {
  std::cout << "leaf count:" << p_result->leaf_count << std::endl;
  std::cout << "node count:" << p_result->node_count << std::endl;
  std::cout << "original size:" << p_result->original_size << std::endl;
  std::cout << "compressed size:" << p_result->compressed_size << std::endl;
  std::cout << "compression ratio:" << (1 - p_result->compression_ratio) * 100
            << "%" << std::endl;
}

Huffman::Huffman()
    : p_builder(std::make_unique<TreeBuilder>(m_info, m_frequencies)) {}

Huffman::~Huffman() {
#ifdef NO_TREE_PRINT
  if (p_stream != nullptr) {
    p_stream->close();
  }
#endif  // NO_TREE_PRINT
  p_result.reset();
}

//=======================================================
TreeBuilder::TreeBuilder(detail::Info& info, FrequencyMap& frequencies)
    : m_info(info), m_frequencies(frequencies) {}

void TreeBuilder::BuildTree() {
  std::priority_queue<Node, std::vector<Node>, Compare> pq;
  {
    for (auto& [key, freq] : m_frequencies) {
      m_info.tree.push_back({
          .self_idx = static_cast<short>(m_info.tree.size()),
          .freq = static_cast<int>(freq),
          .left = -1,
          .right = -1,
          .data = key,
      });
      pq.push(m_info.tree.back());
    }
  }

  // using priority queue to build huffman tree, this is a very easy way to
  //  do till there is more than one node in the queue
  while (pq.size() != 1) {
    // Remove the two nodes of highest priority
    // (lowest frequency) from the queue
    auto left = pq.top();
    pq.pop();
    auto right = pq.top();
    pq.pop();
    // Create a new internal node with these two nodes as children
    // and with frequency equal to the sum of the two nodes'
    // frequencies. Add the new node to the priority queue.
    m_info.tree.push_back({
        .self_idx = static_cast<short>(m_info.tree.size()),
        .freq = static_cast<int>(left.freq + right.freq),
        .left = left.self_idx,
        .right = right.self_idx,
        .data = 0,
    });
    pq.push(m_info.tree.back());
  }

  m_info.tree_size = m_info.tree.size();
}

void TreeBuilder::BuildTree(std::stringstream& ss) {
  for (int i = 0; i < m_info.tree_size; i++) {
    Node node;
    node.self_idx = static_cast<short>(i);  // just for debug
    // read leaf node
    if (i < m_info.leaf_count) {
      // ss.read(reinterpret_cast<char*>(&node.self_idx),
      // sizeof(node.self_idx));

      ss.read(reinterpret_cast<char*>(&node.data), sizeof(node.data));
    } else
    // read none leaf node
    {
      // due to huffman tree might be has 256 leaf nodes, so  all the nodes
      // count will be over 256.therefore, using char to save node's left and
      // right is not enough(just because of the left and right will go over 256
      // and FF)
      ss.read(reinterpret_cast<char*>(&node.left), kChildSize);
      ss.read(reinterpret_cast<char*>(&node.right), kChildSize);
    }
    m_info.tree.push_back(node);
  }
}
std::string TreeBuilder::to_str() {
  std::stringstream ss;
  for (int i = 0; i < m_info.tree_size; i++) {
    // means leaf node
    if (i < m_info.leaf_count) {
      //      ss << m_info.tree[i].data;
      ss.write(reinterpret_cast<char*>(&m_info.tree[i].data),
               sizeof(m_info.tree[i].data));
    } else {
      ss.write(reinterpret_cast<char*>(&m_info.tree[i].left), kChildSize);
      ss.write(reinterpret_cast<char*>(&m_info.tree[i].right), kChildSize);
    }
  }

  return ss.str();
}
