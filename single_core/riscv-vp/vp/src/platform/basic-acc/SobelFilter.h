#ifndef SOBEL_FILTER_H_
#define SOBEL_FILTER_H_
#include <systemc>
#include <cmath>
#include <iomanip>
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "filter_def.h"

struct SobelFilter : public sc_module {
  tlm_utils::simple_target_socket<SobelFilter> tsock;

  sc_fifo<uint8_t> i_plaintext1;
  sc_fifo<unsigned char> i_a;
  sc_fifo<unsigned char> i_b;
  sc_fifo<unsigned char> i_c;
  sc_fifo<unsigned char> i_d;
  sc_fifo<int> o_result;
  sc_time total_start_time;
  sc_time total_run_time;

  SC_HAS_PROCESS(SobelFilter);

  SobelFilter(sc_module_name n): sc_module(n), tsock("t_skt"), base_offset(0) {
    tsock.register_b_transport(this, &SobelFilter::blocking_transport);
    SC_THREAD(do_filter);
  }

  ~SobelFilter() {
	}

  sc_dt::sc_uint<8> RoundKey[176];
  unsigned int base_offset;

  void do_filter(){
    { wait(CLOCK_PERIOD, SC_NS); }
    sc_dt::sc_uint<64> plaintext1, plaintext2;
    sc_dt::sc_uint<64> key1, key2;
    while (true) {
      for(int k = 0; k < 2; k++){
        if(k == 0){
          for(int j = 0; j < 4; j++){
            if(j < 2){
              key1.range((1-j)*32+31,(1-j)*32+24) = i_a.read();
              key1.range((1-j)*32+23,(1-j)*32+16) = i_b.read();
              key1.range((1-j)*32+15,(1-j)*32+8) = i_c.read();
              key1.range((1-j)*32+7,(1-j)*32) = i_d.read();
            }else{
              key2.range((3-j)*32+31,(3-j)*32+24) = i_a.read();
              key2.range((3-j)*32+23,(3-j)*32+16) = i_b.read();
              key2.range((3-j)*32+15,(3-j)*32+8) = i_c.read();
              key2.range((3-j)*32+7,(3-j)*32) = i_d.read();
            }
            //std::cout << "Received Key part" << j << std::endl;
            //wait(CLOCK_PERIOD, SC_NS);
          }
        }else{
          for(int j = 0; j < 4; j++){
            if(j <2){
              plaintext1.range((1-j)*32+31,(1-j)*32+24) = i_a.read();
              plaintext1.range((1-j)*32+23,(1-j)*32+16) = i_b.read();
              plaintext1.range((1-j)*32+15,(1-j)*32+8) = i_c.read();
              plaintext1.range((1-j)*32+7,(1-j)*32) = i_d.read();
            }else{
              plaintext2.range((3-j)*32+31,(3-j)*32+24) = i_a.read();
              plaintext2.range((3-j)*32+23,(3-j)*32+16) = i_b.read();
              plaintext2.range((3-j)*32+15,(3-j)*32+8) = i_c.read();
              plaintext2.range((3-j)*32+7,(3-j)*32) = i_d.read();
            }
            //std::cout << "Received Text part" << j << std::endl;
            //wait(CLOCK_PERIOD, SC_NS);
          }
        }
      }
      sc_dt::sc_uint<64> state1 = plaintext1;
      sc_dt::sc_uint<64> state2 = plaintext2;

      /*std::cout << " key1 = " << std::hex << key1 << std::endl;
      std::cout << " key2 = " << std::hex << key2 << std::endl;
      std::cout << "text1 = " << std::hex << plaintext1 << std::endl;
      std::cout << "text2 = " << std::hex << plaintext2 << std::endl;*/

      total_start_time = sc_time_stamp();
      //encryption
      KeyExpansion(key1,key2);
      AddRoundKey(state1,state2,0);
      for(int round = 1; round < 10; ++round){
        SubBytes(state1,state2);
        ShiftRows(state1,state2);
        MixColumns(state1,state2);
        AddRoundKey(state1,state2,round);
      }

      SubBytes(state1,state2);
      ShiftRows(state1,state2);
      AddRoundKey(state1,state2,10);
      wait(31*CLOCK_PERIOD, SC_NS);
      std::cout << "Finish Process" << std::endl;
      /*std::cout << "Output1 = " << std::hex << state1 << std::endl;
      std::cout << "Output2 = " << std::hex << state2 << std::endl;*/

      /*for(int j = 0; j < 4; j++){
        if(j < 2){
          o_result.write(state2.range(j*32+31,j*32));
        }else{
          o_result.write(state1.range((j-2)*32+31,(j-2)*32));
        }
      }*/
      word buffer;
      for(int j = 0; j < 6; j++){
        if(j < 2){
          buffer.byte[0] = 1;
          buffer.byte[1] = state2.range(j*24+23,j*24+16);
          buffer.byte[2] = state2.range(j*24+15,j*24+8);
          buffer.byte[3] = state2.range(j*24+7,j*24);
          o_result.write(buffer.uint);
        }else if(j == 2){
          buffer.byte[0] = 1;
          buffer.byte[1] = 0;
          buffer.byte[2] = state2.range(j*24+15,j*24+8);
          buffer.byte[3] = state2.range(j*24+7,j*24);
          o_result.write(buffer.uint);
        }else if(j > 2 && j < 5){
          buffer.byte[0] = 1;
          buffer.byte[1] = state1.range((j-3)*24+23,(j-3)*24+16);
          buffer.byte[2] = state1.range((j-3)*24+15,(j-3)*24+8);
          buffer.byte[3] = state1.range((j-3)*24+7,(j-3)*24);
          o_result.write(buffer.uint);
        }else{
          buffer.byte[0] = 2;
          buffer.byte[1] = 0;
          buffer.byte[2] = state1.range((j-3)*24+15,(j-3)*24+8);
          buffer.byte[3] = state1.range((j-3)*24+7,(j-3)*24);
          o_result.write(buffer.uint);
          std::cout << "Latency of this PE= " << sc_time_stamp() - total_start_time << std::endl;
        }
      }
    }
  }

  void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay){
    wait(delay);
    // unsigned char *mask_ptr = payload.get_byte_enable_ptr();
    // auto len = payload.get_data_length();
    tlm::tlm_command cmd = payload.get_command();
    sc_dt::uint64 addr = payload.get_address();
    unsigned char *data_ptr = payload.get_data_ptr();

    addr -= base_offset;


    // cout << (int)data_ptr[0] << endl;
    // cout << (int)data_ptr[1] << endl;
    // cout << (int)data_ptr[2] << endl;
    word buffer;

    switch (cmd) {
      case tlm::TLM_READ_COMMAND:
        //std::cout << "READ" << std::endl;
        switch (addr) {
          case SOBEL_FILTER_RESULT_ADDR:
            buffer.uint = o_result.read();
            break;
          default:
            std::cerr << "READ Error! SobelFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
          }
        data_ptr[0] = buffer.uc[0];
        data_ptr[1] = buffer.uc[1];
        data_ptr[2] = buffer.uc[2];
        data_ptr[3] = buffer.uc[3];
        break;
      case tlm::TLM_WRITE_COMMAND:
        //std::cout << "WRITE" << std::endl;
        switch (addr) {
          case SOBEL_FILTER_R_ADDR:
            i_a.write(data_ptr[0]);
            i_b.write(data_ptr[1]);
            i_c.write(data_ptr[2]);
            i_d.write(data_ptr[3]);
            break;
          default:
            std::cerr << "WRITE Error! SobelFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
        }
        break;
      case tlm::TLM_IGNORE_COMMAND:
        payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
      default:
        payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
      }
      payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
  }
  private:
    void KeyExpansion(sc_dt::sc_uint<64>& key1, sc_dt::sc_uint<64>& key2) { //check
        for (int i = 0; i < 16; ++i) {
        if (i < 8){
          RoundKey[15-i] = key2.range(i*8+7, i*8);
        }else{
          RoundKey[15-i] = key1.range((i-8)*8+7, (i-8)*8);
        }
        }
        sc_dt::sc_uint<8> temp[4];
        int i = 16;
        while (i < 176) {
            for (int j = 0; j < 4; ++j) {
                temp[j] = RoundKey[i - 4 + j];
            }
            if (i % 16 == 0) {
                sc_dt::sc_uint<8> t = temp[0];
                temp[0] = SBox[temp[1]] ^ RCon[i / 16 - 1];
                temp[1] = SBox[temp[2]];
                temp[2] = SBox[temp[3]];
                temp[3] = SBox[t];
            }
            for (int j = 0; j < 4; ++j) {
                RoundKey[i] = RoundKey[i - 16] ^ temp[j];
                ++i;
            }
        }
        //for debug
        /*for (int i = 0; i < 176; i++){
            std::cout << "Round key [" << i << "] =" << std::hex << RoundKey[i] << std::endl;
        }*/
    }

    void AddRoundKey(sc_dt::sc_uint<64>& state1, sc_dt::sc_uint<64>& state2, int round) { //check
        for (int i = 0; i < 16; ++i) {
        if (i < 8){
          state2.range(i*8+7, i*8) = state2.range(i*8+7, i*8) ^ RoundKey[round * 16 + 15 - i];
        }else{
          state1.range((i-8)*8+7, (i-8)*8) = state1.range((i-8)*8+7, (i-8)*8) ^ RoundKey[round * 16 + 15 - i];
        }
        }
        //for debug
        /*std::cout << "AddRoundKey= " << std::hex << state1 << std::endl;
        std::cout << "             " << std::hex << state2 << std::endl;*/
    }

    void SubBytes(sc_dt::sc_uint<64>& state1, sc_dt::sc_uint<64>& state2) { //check
        for (int i = 0; i < 8; ++i) {
            state1.range(i*8+7, i*8) = SBox[state1.range(i*8+7, i*8)];
        state2.range(i*8+7, i*8) = SBox[state2.range(i*8+7, i*8)];
        }
        //for debug
        /*std::cout << "   SubBytes= " << std::hex << state1 << std::endl;
        std::cout << "             " << std::hex << state2 << std::endl;*/
    }

    void ShiftRows(sc_dt::sc_uint<64>& state1, sc_dt::sc_uint<64>& state2) { //check
        sc_dt::sc_uint<64> temp1, temp2;
        // First row (no shift)
        temp1.range(63, 56) = state1.range(63, 56); // temp[0] = state[0]
        temp1.range(31, 24) = state1.range(31, 24); // temp[4] = state[4]
        temp2.range(63, 56) = state2.range(63, 56); // temp[8] = state[8]
        temp2.range(31, 24) = state2.range(31, 24); // temp[12] = state[12]

        // Second row (shift left by 1 byte)
        temp1.range(55, 48) = state1.range(23, 16); // temp[1] = state[5]
        temp1.range(23, 16) = state2.range(55, 48); // temp[5] = state[9]
        temp2.range(55, 48) = state2.range(23, 16); // temp[9] = state[13]
        temp2.range(23, 16) = state1.range(55, 48); // temp[13] = state[1]

        // Third row (shift left by 2 bytes)
        temp1.range(47, 40) = state2.range(47, 40); // temp[2] = state[10]
        temp1.range(15, 8) = state2.range(15, 8); // temp[6] = state[14]
        temp2.range(47, 40) = state1.range(47, 40); // temp[10] = state[2]
        temp2.range(15, 8) = state1.range(15, 8); // temp[14] = state[6]

        // Fourth row (shift left by 3 bytes)
        temp1.range(39, 32) = state2.range(7, 0); // temp[3] = state[15]
        temp1.range(7, 0) = state1.range(39, 32); // temp[7] = state[3]
        temp2.range(39, 32) = state1.range(7, 0); // temp[11] = state[7]
        temp2.range(7, 0) = state2.range(39, 32); // temp[15] = state[11]
        
        state1 = temp1;
      state2 = temp2;

        //for debug
        /*std::cout << "  ShiftRows= " << std::hex << state1 << std::endl;
        std::cout << "             " << std::hex << state2 << std::endl;*/
    }

    void MixColumns(sc_dt::sc_uint<64>& state1, sc_dt::sc_uint<64>& state2) { //check
        sc_dt::sc_uint<8> a, b, c, d;
      for (int i = 0; i < 4; ++i) {
        if (i < 2){
          a = state2.range(i*32+31, i*32+24);
          b = state2.range(i*32+23, i*32+16);
          c = state2.range(i*32+15, i*32+8);
          d = state2.range(i*32+7, i*32);
          state2.range(i*32+31, i*32+24) = GfMul(a, 2) ^ GfMul(b, 3) ^ c ^ d;
          state2.range(i*32+23, i*32+16) = a ^ GfMul(b, 2) ^ GfMul(c, 3) ^ d;
          state2.range(i*32+15, i*32+8)  = a ^ b ^ GfMul(c, 2) ^ GfMul(d, 3);
          state2.range(i*32+7, i*32)     = GfMul(a, 3) ^ b ^ c ^ GfMul(d, 2);
        }else{
          a = state1.range((i-2)*32+31, (i-2)*32+24);
          b = state1.range((i-2)*32+23, (i-2)*32+16);
          c = state1.range((i-2)*32+15, (i-2)*32+8);
          d = state1.range((i-2)*32+7, (i-2)*32);
          state1.range((i-2)*32+31, (i-2)*32+24) = GfMul(a, 2) ^ GfMul(b, 3) ^ c ^ d;
          state1.range((i-2)*32+23, (i-2)*32+16) = a ^ GfMul(b, 2) ^ GfMul(c, 3) ^ d;
          state1.range((i-2)*32+15, (i-2)*32+8)  = a ^ b ^ GfMul(c, 2) ^ GfMul(d, 3);
          state1.range((i-2)*32+7, (i-2)*32)     = GfMul(a, 3) ^ b ^ c ^ GfMul(d, 2);
        }
        }
        //for debug
        /*std::cout << " MixColumns= " << std::hex << state1 << std::endl;
        std::cout << "             " << std::hex << state2 << std::endl;*/
    }

    sc_dt::sc_uint<8> GfMul(sc_dt::sc_uint<8> a, sc_dt::sc_uint<8> b) {
        sc_dt::sc_uint<8> p = 0;
        sc_dt::sc_uint<8> hi_bit_set;
        for (int counter = 0; counter < 8; ++counter) {
            if (b & 1) {
                p ^= a;
            }
            hi_bit_set = (a & 0x80);
            a <<= 1;
            if (hi_bit_set) {
                a ^= 0x1b;
            }
            b >>= 1;
        }
        return p;
    }

    const uint8_t SBox[256] = {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
        0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
        0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
        0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
        0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
        0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
        0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
        0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
        0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
        0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
        0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
        0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
        0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
        0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
        0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
        0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
        0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
    };

    const uint8_t RCon[255] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
        0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f,
        0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4,
        0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72,
        0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a,
        0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74,
        0xe8, 0xcb, 0x8d, 0x01, 0x00, 0x00, 0x00, 0x00
    };

};
#endif
