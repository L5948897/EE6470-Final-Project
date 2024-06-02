#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <systemc>
using namespace sc_core;

#include "Testbench.h"
#ifndef NATIVE_SYSTEMC
#include "SobelFilter_wrap.h"
#else
#include "SobelFilter.h"
#endif

class System: public sc_module
{
public:
	SC_HAS_PROCESS( System );
	System( sc_module_name n );
	~System();
private:
  Testbench tb;
#ifndef NATIVE_SYSTEMC
	SobelFilter_wrapper sobel_filter;
#else
	SobelFilter sobel_filter;
#endif
	sc_clock clk;
	sc_signal<bool> rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_uint<64> > plaintext1;
	cynw_p2p< sc_dt::sc_uint<64> > plaintext2;
	cynw_p2p< sc_dt::sc_uint<64> > key1;
	cynw_p2p< sc_dt::sc_uint<64> > key2;
	cynw_p2p< sc_dt::sc_uint<64> > ciphertext1;
	cynw_p2p< sc_dt::sc_uint<64> > ciphertext2;
#else
	sc_fifo< sc_dt::sc_uint<64> > plaintext1;
	sc_fifo< sc_dt::sc_uint<64> > plaintext2;
	sc_fifo< sc_dt::sc_uint<64> > key1;
	sc_fifo< sc_dt::sc_uint<64> > key2;
	sc_fifo< sc_dt::sc_uint<64> > ciphertext1;
	sc_fifo< sc_dt::sc_uint<64> > ciphertext2;
#endif
};
#endif
