#ifndef SOBEL_FILTER_H_
#define SOBEL_FILTER_H_
#include <systemc>
using namespace sc_core;

#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif

class SobelFilter: public sc_module
{
public:
	sc_in_clk i_clk;
	sc_in < bool >  i_rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_uint<64> >::in i_plaintext1;
	cynw_p2p< sc_dt::sc_uint<64> >::in i_plaintext2;
	cynw_p2p< sc_dt::sc_uint<64> >::in i_key1;
	cynw_p2p< sc_dt::sc_uint<64> >::in i_key2;
	cynw_p2p< sc_dt::sc_uint<64> >::out o_ciphertext1;
	cynw_p2p< sc_dt::sc_uint<64> >::out o_ciphertext2;
#else
	sc_fifo_in< sc_dt::sc_uint<64> > i_plaintext1;
	sc_fifo_in< sc_dt::sc_uint<64> > i_plaintext2;
	sc_fifo_in< sc_dt::sc_uint<64> > i_key1;
	sc_fifo_in< sc_dt::sc_uint<64> > i_key2;
	sc_fifo_out< sc_dt::sc_uint<64> > o_ciphertext1;
	sc_fifo_out< sc_dt::sc_uint<64> > o_ciphertext2;
#endif

	SC_HAS_PROCESS( SobelFilter );
	SobelFilter( sc_module_name n );
	~SobelFilter();

private:
	sc_dt::sc_uint<8> RoundKey[176];
	static const uint8_t SBox[256];
	void KeyExpansion(sc_dt::sc_uint<64>& key1, sc_dt::sc_uint<64>& key2);
	void AddRoundKey(sc_dt::sc_uint<64>& state1, sc_dt::sc_uint<64>& state2, int round);
	void SubBytes(sc_dt::sc_uint<64>& state1, sc_dt::sc_uint<64>& state2);
	void ShiftRows(sc_dt::sc_uint<64>& state1, sc_dt::sc_uint<64>& state2);
	void MixColumns(sc_dt::sc_uint<64>& state1, sc_dt::sc_uint<64>& state2);
	sc_dt::sc_uint<8> GfMul(sc_dt::sc_uint<8> a, sc_dt::sc_uint<8> b);
	
	void do_filter();
};
#endif
