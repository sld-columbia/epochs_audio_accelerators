//#include <ac_sysc_trace.h>
#include "PingPong.h"

#include <mc_scverify.h>

class testbench : public sc_module
{
public:
  CCS_DESIGN(PingPong) CCS_INIT_S1(dut1);

  sc_clock clk;
  SC_SIG(bool, rst_bar);
  typedef ac_int<16> T;

  Connections::Combinational<T>        CCS_INIT_S1(out1);
  Connections::Combinational<T>        CCS_INIT_S1(in1);

  SC_CTOR(testbench)
    :   clk("clk", 1, SC_NS, 0.5,0,SC_NS,true) {
    //Connections::set_sim_clk(&clk);
    //sc_object_tracer<sc_clock> trace_clk(clk);

    dut1.clk(clk);
    dut1.rst_bar(rst_bar);
    dut1.out1(out1);
    dut1.in1(in1);

    SC_CTHREAD(reset, clk);

    SC_THREAD(stim);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);

    SC_THREAD(resp);
    sensitive << clk.posedge_event();
    async_reset_signal_is(rst_bar, false);
  }

  void stim() {
    in1.ResetWrite();
    wait(20, SC_NS);
    
    CCS_LOG("Stimulus started");
    wait();

    for (int i = 0; i < 40; i++) {
      in1.Push(i);
    }
    
    wait(1000, SC_NS);

    sc_stop();
    wait();
  }

  void resp() {
    out1.ResetRead();
    wait();

    while (1) {
      CCS_LOG("TB resp sees: " << std::dec << out1.Pop());
    }
  }

  void reset() {
    rst_bar.write(0);
    wait(5);
    rst_bar.write(1);
    wait();
  }
};

int sc_main(int argc, char **argv)
{
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
  //sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

  testbench my_testbench("my_testbench");
  //trace_hierarchy(&top, trace_file_ptr);
  sc_start();
  if (sc_report_handler::get_count(SC_ERROR) > 0) {
    std::cout << "Simulation FAILED" << std::endl;
    return -1;
  }
  std::cout << "Simulation PASSED" << std::endl;
  return 0;
}

