# Huffman-coding

#### About:
This is an implementation of a static Huffman coding and decoding algorithm in C programming language. Static means that the input must be read twice and the encoding tree is transmitted with the encoded content of the input.
It is assumed that the input contains at most 256 unique characters.
Encoder uses a binary min-heap to construct an encoding tree which is then used to encode the input.
Encoded file consists of encoded encoding tree in the header followed by the encoded content.
Decoder first reconstructs the encoding tree and then uses it to decode the content.



#### Project structure:
- driver: contains the program with main function, main.c
- lib: contains  header files and libraries
  - api: contains header files and source files that implement functions declared in headers
  - src: contains the main library for Huffman encoding and decoding algorithms
- compile: contains the Makefile
- obj: after running make, object files are placed in here
- bin: after running make, an executable program is placed in here 

#### Usage: 

    1. run the make command in the compile directory where Makefile is located
    2. it should create an executable called huff in the bin directory
    3. run the huff executable
    
## Command line interface
    
#### Command line options
   - -c, encoding mode
   - -d, decoding mode
   - -i, optional input file
   - -o, optional output file
   - -g, launch GUI (not yet implemented)
   - -s, print stats/info
   - -h, display help

Input file name argument is required. You can specify it as a -i followed by a file name or just as a standard argument. 
Example: ./huff -c -i input.txt or ./huff -c input.txt
Output file name argument is optional. You can specify it as a -o followed by a file name. If you don`t:
- in encoding mode, file name will be the same as input file name but with .huff extension
- In decoding mode, file name will be the same as input file name but without .huff extension

## Implementation
#### Encoding
  Implementation of encoding is static meaning the encoding tree is built after first reading of the input.
  First scan of the input file results in a map of unique characters that appear in the file. It is assumed that the input contains at most 256 unique characters therefore the map is of size 256 + 1 for the pseudo EOF character.
  Every new not yet seen character is inserted at index in the map that is its ascii decimal value for example: 
  'a'...97 is inserted at tndex 97, 'b' at 98, ...; This approach eliminates the searching of each character later on when the encoded values get written in the ouput file and checking if the just read character already exists in the map.
  Encoding tree is build using the deque operation on priority queue which is implemented as binary min-heap. Dequeue returns the character with the smallest occurance probability.
  When the encoding tree is built, encoding of the leaf nodes takes place. Left branch in the subtree contributes a bit 0 to the character code and the right branch bit 1. At run-time, the codes are stored as strings. 
  When the assigning of codes to the characters is complete, its time to encode the input file.
  First we need to encode the encoding tree so the decoder has the exact same tree as encoder.
  Tree is encoded with pre-order traversal. Non-leaf nodes are represented with bit 0, while leaf nodes are represented as bit 1 followed by a 16-bits that represent the value of a character that this leaf holds.
  After the encoded tree the encoded content is written.
  That requires second traversal of the input file. Instead of characters found in the input, their codes are written in the output file. Codes are shifted into a 32-bit bit-buffer and when its full, we write it into the encoded file.
  At the end, the pseudo EOF character code gets written. 
  
  

#### Decoding
  First byte n in the encoded file represents the number of encoded encoding tree blocks, so n 32-bit blocks are read into memory. Decoder first reconstructs the encoding tree so it can decode the following encoded content.
Encoded content is read bit by bit and according to the rule of bit 1 (meaning move right) and bit 0 (meaning move left), the decoder moves throught the tree from the root down to the leaf nodes. When the code brings it to the leaf node, encoder  writes the character in that node to the output buffer.



