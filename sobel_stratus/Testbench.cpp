#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

#include "Testbench.h"

Testbench::Testbench(sc_module_name n) : sc_module(n) {
  SC_THREAD(feed_rgb);
  sensitive << i_clk.pos();
  dont_initialize();
  SC_THREAD(fetch_result);
  sensitive << i_clk.pos();
  dont_initialize();
}

Testbench::~Testbench() {
	//cout<< "Max txn time = " << max_txn_time << endl;
	//cout<< "Min txn time = " << min_txn_time << endl;
	//cout<< "Avg txn time = " << total_txn_time/n_txn << endl;
	cout << "Total run time = " << total_run_time << endl;
}

void Testbench::read_file(void) {
  string key;
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
  string key1_str = key.substr(0,16);
  string key2_str = key.substr(16,16);
  key1 = stoull(key1_str, nullptr, 16);
  key2 = stoull(key2_str, nullptr, 16);
  for(int i =0; i < 10; i++){
    //cout << "input text[" << i << "] = " << text[i] << endl;
    string plaintext_str = text[i];
    string plaintext1_str = plaintext_str.substr(0,16);
    string plaintext2_str = plaintext_str.substr(16,16);
    plaintext1[i] = stoull(plaintext1_str, nullptr, 16);
    plaintext2[i] = stoull(plaintext2_str, nullptr, 16);
  }
}

void Testbench::feed_rgb() {

#ifndef NATIVE_SYSTEMC
	o_plaintext1.reset();
  o_plaintext2.reset();
  o_key1.reset();
  o_key2.reset();
#endif
	o_rst.write(false);
	wait(5);
	o_rst.write(true);
	wait(1);
	total_start_time = sc_time_stamp();

  read_file();
  /*plaintext1 = 0x3243f6a8885a308d;
  plaintext2 = 0x313198a2e0370734;
  key1 = 0x2b7e151628aed2a6;
  key2 = 0xabf7158809cf4f3c;*/
  cout << "       Key1= " << hex << key1 << endl;
  cout << "       Key2= " << hex << key2 << endl;
  for(int i = 0; i < 10; i++){
    cout << endl;
    cout << " Plaintext1= " << hex << plaintext1[i] << endl;
    cout << " Plaintext2= " << hex << plaintext2[i] << endl;
#ifndef NATIVE_SYSTEMC
    o_plaintext1.put(plaintext1[i]);
    o_plaintext2.put(plaintext2[i]);
    o_key1.put(key1);
    o_key2.put(key2);
#else
    o_plaintext1.write(plaintext1[i]);
    o_plaintext2.write(plaintext2[i]);
    o_key1.write(key1);
    o_key2.write(key2);
#endif
  }
}

void Testbench::fetch_result() {
  sc_dt::sc_uint<64> ciphertext1[10], ciphertext2[10];
  sc_dt::sc_uint<64> checktext1[10] = {0x3925841d02dc09fb,
                                       0x8df4e9aac5c7573a,
                                       0x50fe67cc996d32b6,
                                       0xd44f0b792fd3b7c1,
                                       0x2f49671c7ab81b2f,
                                       0xf790f370ebf93852,
                                       0xfa1be9bcacfb0ade,
                                       0x99701d39000bac62,
                                       0x7df76b0c1ab899b3,
                                       0xf9eb38f33b842646};
  sc_dt::sc_uint<64> checktext2[10] = {0xdc118597196a0b32,
                                       0x27d8d055d6e4d64b,
                                       0xda0937e99bafec60,
                                       0x02a300501dba089c,
                                       0x435d9b650e35b8c1,
                                       0x651a50ec3602777b,
                                       0x7ac7cedcc228ca87,
                                       0xb50842e85791f1b0,
                                       0x3e42f047b91b546f,
                                       0x9193f711d7716811};
#ifndef NATIVE_SYSTEMC
	i_ciphertext1.reset();
  i_ciphertext2.reset();
#endif
	wait(5);
	wait(1);
#ifndef NATIVE_SYSTEMC
  for(int i = 0; i < 10; i++){
    ciphertext1[i] = i_ciphertext1.get();
    ciphertext2[i] = i_ciphertext2.get();
#else
    ciphertext1[i] = i_ciphertext1.read();
    ciphertext2[i] = i_ciphertext2.read();
#endif
  }
  for (int i = 0; i < 10; i++){
    cout << endl;
    cout << "Ciphertext1= " << hex << ciphertext1[i] << endl;
    cout << "Ciphertext2= " << hex << ciphertext2[i] << endl;
    if(ciphertext1[i] == checktext1[i] && ciphertext2[i] == checktext2[i]){
      cout << "Correct!" <<endl;
    }else{
      cout << "Wrong!" <<endl;
    }
  }

	total_run_time = sc_time_stamp() - total_start_time;
  sc_stop();
}
