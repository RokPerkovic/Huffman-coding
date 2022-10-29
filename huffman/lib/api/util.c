#include "util.h"

void dec_to_bin(char *bin, unsigned int dec, int size){
    for (int i = size - 1; i >= 0; --i){
    	bin[(size - 1) - i] = (dec & (1 << i)) ? '1' : '0';
    }
    bin[size + 1] = '\0';
}
