#include "string"
#include "string.h"
#include "cassert"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include <cmath>
#include <iostream>
//#include <systemc>

#define TEXT 10

union word {
  int sint;
  unsigned int uint;
  //sc_dt::sc_uint<8> byte[4];
  uint8_t byte[4];
  unsigned char uc[4];
  /*// Explicitly define the default constructor
  word() : sint(0), uint(0) {
    for (int i = 0; i < 4; ++i) {
      byte[i] = 0;
      uc[i] = 0;
    }
  }
  // Explicitly define the destructor
  ~word() {}*/
};

// Sobel Filter ACC
static char* const SOBELFILTER_START_ADDR = reinterpret_cast<char* const>(0x73000000);
static char* const SOBELFILTER_READ_ADDR  = reinterpret_cast<char* const>(0x73000004);

// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;

//bool _is_using_dma = false;
bool _is_using_dma = true;

void write_data_to_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(buffer);
    *DMA_DST_ADDR = (uint32_t)(ADDR);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(ADDR);
    *DMA_DST_ADDR = (uint32_t)(buffer);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

int main(int argc, char *argv[]) {
  std::string key;
  /*ifstream fin;
  fin.open("text.txt");
  if (fin.fail()){
    cout <<"Input file opening fail."<<endl;
    return;
  }
  fin >> key;
  fin >> text;*/
  key =  "2b7e151628aed2a6abf7158809cf4f3c";
  const char* text[10] = {"3243f6a8885a308d313198a2e0370734",
                          "00112233445566778899aabbccddeeff",
                          "000102030405060708090a0b0c0d0e0f",
                          "0123456789abcdef0123456789abcdef",
                          "ffeeddccbbaa99887766554433221100",
                          "a156c897e564613c564a897b564dfa45",
                          "a7894e45613c4fd14569231456159984",
                          "1231adf489ac456e8456d98b46489b56",
                          "00000000000000000000000000000000",
                          "89765465561316549879561316549746"};
  //cout << "input key  = " << key << endl;
  std::string key1_str = key.substr(0,16);
  std::string key2_str = key.substr(16,16);
  uint64_t key1, key2;
  uint64_t plaintext1[10], plaintext2[10];
  uint64_t ciphertext1[10], ciphertext2[10];
  for (int j = 0; j < TEXT; j++){
      ciphertext1[j] = 0;
      ciphertext2[j] = 0;
    }
  key1 = stoull(key1_str, nullptr, 16);
  key2 = stoull(key2_str, nullptr, 16);
  for(int i =0; i < 10; i++){
    //cout << "input text[" << i << "] = " << text[i] << endl;
    std::string plaintext_str = text[i];
    std::string plaintext1_str = plaintext_str.substr(0,16);
    std::string plaintext2_str = plaintext_str.substr(16,16);
    plaintext1[i] = stoull(plaintext1_str, nullptr, 16);
    plaintext2[i] = stoull(plaintext2_str, nullptr, 16);
  }

  word buffer;
  //unsigned char  buffer[4] = {0};
  word data;
  int total;
  printf("===========================\n");
  printf("Start processing...\n");
  printf("===========================\n\n");
  for (int i = 0; i < TEXT; i++){
    //send
    //printf("Sending no.%d data...\n", i);
    for (int k = 0; k < 2; k++){
      if (k == 0){
        printf("Sending no.%d key...\n", i);
        for (int j = 0; j < 4; j++){ //send key
          //printf("Sending part%d of key...\n", j);
          buffer.byte[0] = stoul(key.substr(j*8,2), nullptr, 16);
          buffer.byte[1] = stoul(key.substr(j*8+2,2), nullptr, 16);
          buffer.byte[2] = stoul(key.substr(j*8+4,2), nullptr, 16);
          buffer.byte[3] = stoul(key.substr(j*8+6,2), nullptr, 16);
          write_data_to_ACC(SOBELFILTER_START_ADDR, buffer.uc, 4);
        }
      } else {
        printf("Sending no.%d text...\n", i);
        for (int j = 0; j < 4; j++){ //send plaintext
          //printf("Sending part%d of plaintext...\n", j);
          std::string plaintext_str = text[i];
          buffer.byte[0] = stoul(plaintext_str.substr(j*8,2), nullptr, 16);
          buffer.byte[1] = stoul(plaintext_str.substr(j*8+2,2), nullptr, 16);
          buffer.byte[2] = stoul(plaintext_str.substr(j*8+4,2), nullptr, 16);
          buffer.byte[3] = stoul(plaintext_str.substr(j*8+6,2), nullptr, 16);
          write_data_to_ACC(SOBELFILTER_START_ADDR, buffer.uc, 4);
        }
      }
    }
    //receive
    printf("Receiving no.%d text...\n\n", i);
    /*for (int j = 0; j < 4; j++){
      read_data_from_ACC(SOBELFILTER_READ_ADDR, buffer.uc, 4);
      memcpy(data.uc, buffer.uc, 4);
      //std::cout << "Received data = " << std::hex << data.uint << std::endl;
      if (j < 2){
        ciphertext2[i] += static_cast<long long>(data.uint)<<(32*j);
      } else {
        ciphertext1[i] += static_cast<long long>(data.uint)<<(32*(j-2));
      }
    }*/
    int c = 0;
    while(c < 6){
      read_data_from_ACC(SOBELFILTER_READ_ADDR, buffer.uc, 4);
      memcpy(data.uc, buffer.uc, 4);
      //std::cout << "Received data = " << std::hex << data.uint << std::endl;
      //std::cout << "c = " << std::hex << c << std::endl;
      if (c < 2){
        if (data.byte[0] == 1) { //valid
          ciphertext2[i] += static_cast<long long>(data.byte[1])<<(24*c+16);
          ciphertext2[i] += static_cast<long long>(data.byte[2])<<(24*c+8);
          ciphertext2[i] += static_cast<long long>(data.byte[3])<<(24*c);
          c++;
        }
      } else if (c == 2) {
        if (data.byte[0] == 1) { //valid
          ciphertext2[i] += static_cast<long long>(data.byte[2])<<(24*c+8);
          ciphertext2[i] += static_cast<long long>(data.byte[3])<<(24*c);
          c++;
        }
      } else if (c > 2 && c < 5) {
        if (data.byte[0] == 1) { //valid
          ciphertext1[i] += static_cast<long long>(data.byte[1])<<(24*(c-3)+16);
          ciphertext1[i] += static_cast<long long>(data.byte[2])<<(24*(c-3)+8);
          ciphertext1[i] += static_cast<long long>(data.byte[3])<<(24*(c-3));
          c++;
        }
      } else {
        if (data.byte[0] == 2) { //done
          ciphertext1[i] += static_cast<long long>(data.byte[2])<<(24*(c-3)+8);
          ciphertext1[i] += static_cast<long long>(data.byte[3])<<(24*(c-3));
          c++;
        }
      }
    }
  }
  printf("===========================\n");
  printf("Finished All Encrpytion...\n");
  printf("===========================\n");
  printf("Start Checking...\n\n");
  uint64_t checktext1[10] = {0x3925841d02dc09fb,
                            0x8df4e9aac5c7573a,
                            0x50fe67cc996d32b6,
                            0xd44f0b792fd3b7c1,
                            0x2f49671c7ab81b2f,
                            0xf790f370ebf93852,
                            0xfa1be9bcacfb0ade,
                            0x99701d39000bac62,
                            0x7df76b0c1ab899b3,
                            0xf9eb38f33b842646};
  uint64_t checktext2[10] = {0xdc118597196a0b32,
                            0x27d8d055d6e4d64b,
                            0xda0937e99bafec60,
                            0x02a300501dba089c,
                            0x435d9b650e35b8c1,
                            0x651a50ec3602777b,
                            0x7ac7cedcc228ca87,
                            0xb50842e85791f1b0,
                            0x3e42f047b91b546f,
                            0x9193f711d7716811};
  for (int i = 0; i < TEXT; i++){
    std::cout << "Ciphertext1 = " << std::hex << ciphertext1[i] << std::endl;
    std::cout << "Ciphertext2 = " << std::hex << ciphertext2[i] << std::endl;
    if(ciphertext1[i] == checktext1[i] && ciphertext2[i] == checktext2[i]){
      printf("Correct!\n\n");
    }else{
      printf("Wrong!\n\n");
    }
  }
}
