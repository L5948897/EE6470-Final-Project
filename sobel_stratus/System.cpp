#include "System.h"
System::System( sc_module_name n ): sc_module( n ), 
	tb("tb"), sobel_filter("sobel_filter"), clk("clk", CLOCK_PERIOD, SC_NS), rst("rst")
{
	tb.i_clk(clk);
	tb.o_rst(rst);
	sobel_filter.i_clk(clk);
	sobel_filter.i_rst(rst);
	tb.o_plaintext1(plaintext1);
	tb.o_plaintext2(plaintext2);
	tb.o_key1(key1);
	tb.o_key2(key2);
	tb.i_ciphertext1(ciphertext1);
	tb.i_ciphertext2(ciphertext2);
	sobel_filter.i_plaintext1(plaintext1);
	sobel_filter.i_plaintext2(plaintext2);
	sobel_filter.i_key1(key1);
	sobel_filter.i_key2(key2);
	sobel_filter.o_ciphertext1(ciphertext1);
	sobel_filter.o_ciphertext2(ciphertext2);
}

System::~System() {
}
