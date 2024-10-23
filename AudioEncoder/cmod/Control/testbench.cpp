/* Testing design */

#include "Control.h"
#include <systemc.h>
#include <mc_scverify.h>

#define NVHLS_VERIFY_BLOCKS (Control)
#include <nvhls_verify.h>
#include <testbench/nvhls_rand.h>
using namespace::std;


SC_MODULE (Source) {
  sc_in <bool> clk;
  sc_in <bool> rst;

  sc_out<bool> acc_start;    // start signal
  sc_in<bool> acc_done;    // done signal


  Connections::In<bool>     sync1;
  Connections::In<bool>     sync2;
  Connections::Out<bool>    sync3;


  SC_CTOR(Source) {
    SC_THREAD(run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    acc_start.write(0);
    sync1.Reset();
    sync2.Reset();
    sync3.Reset();

    wait(100, SC_NS);

    // start signal 
    acc_start.write(1);
    cout << "@" << sc_time_stamp() << " acc_start.write(1) " << endl;
    wait();
    wait();
    acc_start.write(0);
    cout << "@" << sc_time_stamp() << " acc_start.write(0) " << endl;


    sync1.Pop();
    wait();
    cout << "@" << sc_time_stamp() << " sync1.Pop() " << endl;
    sync2.Pop();
    cout << "@" << sc_time_stamp() << " sync1.Pop() " << endl;
    wait();
    wait();
    wait();

    sync3.Push(1);
    cout << "@" << sc_time_stamp() << " sync3.Push(1) " << endl;
    wait();
    wait();
    wait();

    while (acc_done.read() == 0) {
      wait();
    }
    cout << "@" << sc_time_stamp() << " acc_done.read() == 1 " << endl;
    wait(5);
  }
};


SC_MODULE (testbench) {
  sc_clock clk;
  sc_signal<bool> rst;


  sc_signal<bool> acc_start;    // start signal
  sc_signal<bool> acc_done;    // done signal
  Connections::Combinational<bool>    sync1;
  Connections::Combinational<bool>    sync2;
  Connections::Combinational<bool>    sync3;
  
  NVHLS_DESIGN(Control) dut;
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
    dut.acc_start(acc_start);
    dut.acc_done(acc_done);
    dut.sync1(sync1);
    dut.sync2(sync2);
    dut.sync3(sync3);

    src.clk(clk);
    src.rst(rst);
    src.acc_start(acc_start);
    src.acc_done(acc_done);
    src.sync1(sync1);
    src.sync2(sync2);
    src.sync3(sync3);

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