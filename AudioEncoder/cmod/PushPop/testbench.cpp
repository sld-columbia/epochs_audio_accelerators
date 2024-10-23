/* Testing design */

#include "PushPop.h"
#include <systemc.h>
#include <mc_scverify.h>

#define NVHLS_VERIFY_BLOCKS (PushPop)
#include <nvhls_verify.h>
#include <testbench/nvhls_rand.h>
using namespace::std;


SC_MODULE (Source) {
  sc_in <bool> clk;
  sc_in <bool> rst;

  //Connections::SyncOut  sync1;
  Connections::Out<bool>  sync1;

  Connections::Out<ac_int<64,true>> data_in;
  Connections::In<ac_int<64,true>> data_out;


  SC_CTOR(Source) {
    SC_THREAD(run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    // generate synthetic values

    //sync1.reset_sync_out();
    sync1.Reset();
    data_in.Reset();
    data_out.Reset();
    
    // Wait for initial reset
    wait(100, SC_NS);
    wait(20, SC_NS);
    
    cout << "@" << sc_time_stamp() << " Start Computation " << endl ;
    // start computation 
    //sync1.sync_out();
    bool tmp = 1;
    sync1.Push(tmp);

    wait(10, SC_NS);
    for (int i = 0; i < 1024; i++) {
      ac_int<64,true> in = i;

      data_in.Push(in);
      wait();
      ac_int<64,true> out = data_out.Pop();
      cout << "@" << sc_time_stamp() << " Pop out = " << out << endl;
      wait();
    }


    wait(5); 
  }// void run()
};

SC_MODULE (testbench) {
  sc_clock clk;
  sc_signal<bool> rst;

  //Connections::SyncChannel  sync1; 
  Connections::Combinational<bool> sync1;
  Connections::Combinational<ac_int<64,true>> data_in;
  Connections::Combinational<ac_int<64,true>> data_out;

  NVHLS_DESIGN(PushPop) dut;
  Source src;

  SC_HAS_PROCESS(testbench);
  testbench(sc_module_name name)
  : sc_module(name),
    clk("clk", 1, SC_NS, 0.5,0,SC_NS,true),
//    clk("clk", 2, SC_NS, 0.5,0,SC_NS,true),
    rst("rst"),
    dut("dut"),
    src("src")
  {
    
    dut.clk(clk);
    dut.rst(rst);
    dut.sync1(sync1);
    dut.data_in(data_in);
    dut.data_out(data_out);



    src.clk(clk);
    src.rst(rst);
    src.sync1(sync1);
    src.data_in(data_in);
    src.data_out(data_out);
    
    SC_THREAD(run);
  }

  void run() {
    rst = 1;
    wait(10, SC_NS);
    rst = 0;
    cout << "@" << sc_time_stamp() << " Asserting Reset " << endl ;
    wait(10, SC_NS);
    cout << "@" << sc_time_stamp() << " Deasserting Reset " << endl ;
    rst = 1;
    wait(10000,SC_NS);
    cout << "@" << sc_time_stamp() << " Stop " << endl ;
    sc_stop();
  }
};

int sc_main(int argc, char *argv[])
{
    //nvhls::set_random_seed();
    testbench my_testbench("my_testbench");
    sc_start();
    cout << "CMODEL PASS" << endl;
    return 0;
};