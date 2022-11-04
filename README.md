# Huffman-coding

#### About:
This is a implementation of a static Huffman coding and decoding algorithm in C programming language. Static means that the input must be read twice and the encoding tree is transmitted with the encoded content of the input.
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
   - -s, print stats/info (not yet implemented)
   - -h, display help

Input file name argument is required. You can specify it as a -i followed by a file name or just as a standard argument. 
Example: ./huff -c -i input.txt or ./huff -c input.txt
Output file name argument is optional. You can specify it as a -o followed by a file name. If you don`t:
- in encoding mode, file name will be the same as input file name but with .huff extension
- In decoding mode, file name will be the same as input file name but without .huff extension

## Implementation
#### Encoding
  aaaaa

#### Decoding
  bbbbb




