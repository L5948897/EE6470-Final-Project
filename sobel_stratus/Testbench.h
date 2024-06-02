#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include <string>
using namespace std;

#include <systemc>
using namespace sc_core;

#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif

class Testbench : public sc_module {
public:
	sc_in_clk i_clk;
	sc_out < bool >  o_rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_uint<64> >::base_out o_plaintext1;
  cynw_p2p< sc_dt::sc_uint<64> >::base_out o_plaintext2;
  cynw_p2p< sc_dt::sc_uint<64> >::base_out o_key1;
  cynw_p2p< sc_dt::sc_uint<64> >::base_out o_key2;
	cynw_p2p< sc_dt::sc_uint<64> >::base_in i_ciphertext1;
  cynw_p2p< sc_dt::sc_uint<64> >::base_in i_ciphertext2;
#else
	sc_fifo_out< sc_dt::sc_uint<64> > o_plaintext1;
  sc_fifo_out< sc_dt::sc_uint<64> > o_plaintext2;
  sc_fifo_out< sc_dt::sc_uint<64> > o_key1;
  sc_fifo_out< sc_dt::sc_uint<64> > o_key2;
	sc_fifo_in< sc_dt::sc_uint<64> > i_ciphertext1;
  sc_fifo_in< sc_dt::sc_uint<64> > i_ciphertext2;
#endif

  SC_HAS_PROCESS(Testbench);

  Testbench(sc_module_name n);
  ~Testbench();

private:
  sc_dt::sc_uint<64> key1, key2;
  sc_dt::sc_uint<64> plaintext1[10];
  sc_dt::sc_uint<64> plaintext2[10];
	sc_time total_start_time;
	sc_time total_run_time;

  void read_file(void);
  void feed_rgb();
	void fetch_result();
};
#endif
