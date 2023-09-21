# Huffman
Simply Huffman Compressor and Decompressor 


## how to use
### Encode
```cpp
std::string buffer  = "This is a test case";
std::string encoded_buffer ;
Huffman huffman;
encoded_buffer = huffman.Encode(buffer);
```

### Decode
```cpp
std::string buffer ="the Encoded data buffer";
std::string decode_buffer ; // the decoded buffer 
Huffman huffman;
decode_buffer = huffman.Encode(buffer);

```
