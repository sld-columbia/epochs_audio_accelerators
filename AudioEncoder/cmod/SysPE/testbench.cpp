/* Testing design */

#include "SysPE.h"
#include <systemc.h>
#include <mc_scverify.h>

#define NVHLS_VERIFY_BLOCKS (SysPE)
#include <nvhls_verify.h>
#include <testbench/nvhls_rand.h>
using namespace::std;


SC_MODULE (Source) {
  Connections::Out<NVINT8> act_in;
  Connections::Out<NVINT8> weight_in;
  Connections::Out<NVINT32> accum_in;
  Connections::In<NVINT32>  accum_out;
  Connections::In<NVINT8>  act_out;
  Connections::In<NVINT8>  weight_out;
  sc_in <bool> clk;
  sc_in <bool> rst;
  
  bool start_src;  
  vector<NVINT8> act_list{0, -1, 3, -7, 15, -31, 63, -127};
  vector<NVINT32> accum_list{0, -10, 30, -70, 150, -310, 630, -1270};
  NVINT8 weight_data = 10;
  NVINT32 accum_init = 0;
  NVINT8 act_out_src;
  NVINT32 accum_out_src;

  SC_CTOR(Source) {
    SC_THREAD(run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
    SC_THREAD(pop_result);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    NVINT8 _act;
    NVINT32 _acc;
    act_in.Reset();
    weight_in.Reset();
    accum_in.Reset();

    // Wait for initial reset
    wait(100, SC_NS);
    // Write wait to PE
    weight_in.Push(weight_data);
    wait(5);

    for (int i=0; i< (int) act_list.size(); i++) {
        _act = act_list[i];
        _acc = accum_list[i];
        act_in.Push(_act);
        wait();
        wait();
        wait();               
        accum_in.Push(_acc);
        wait(); 
        wait();    
    } // for  
    wait(5); 
  }// void run()
  void pop_result() {
    weight_out.Reset();
    act_out.Reset();
    accum_out.Reset();
    wait(20, SC_NS);

    unsigned int i = 0, j = 0;
    bool correct = 1;
    while (1) {    
        NVINT8 tmp;
        if (weight_out.PopNB(tmp)) {
           cout << sc_time_stamp() << ": Received Output Weight:" << " \t " << tmp << endl;
        }
        if (act_out.PopNB(act_out_src)) { 
           cout << sc_time_stamp() << ": Received Output Activation:" << " \t " << act_out_src << "\t| Reference \t" << act_list[i] << endl;
           correct &= (act_list[i] == act_out_src);
	           i++;
        }
        if (accum_out.PopNB(accum_out_src)) {
           int acc_ref = accum_list[j] + act_list[j]*weight_data;
           cout << sc_time_stamp() << ": Received Accumulated Output:" << "\t " << accum_out_src << "\t| Reference \t" << acc_ref << endl;
           correct &= (acc_ref == accum_out_src);
	         j++;
        }
        wait();
      if (i == act_list.size() && j == act_list.size()) break;  
    }// while
    
   if (correct == 1) cout << "Implementation Correct :)" << endl;
   else  cout << "Implementation Incorrect (:" << endl;
  }// void pop_result()
};

SC_MODULE (testbench) {
  sc_clock clk;
  sc_signal<bool> rst;

  Connections::Combinational<NVINT8> act_in;
  Connections::Combinational<NVINT8> act_out;
  Connections::Combinational<NVINT8> weight_in;
  Connections::Combinational<NVINT8> weight_out;
  Connections::Combinational<NVINT32> accum_in;
  Connections::Combinational<NVINT32> accum_out;

  NVHLS_DESIGN(SysPE) PE;
  Source src;

  SC_HAS_PROCESS(testbench);
  testbench(sc_module_name name)
  : sc_module(name),
    clk("clk", 1, SC_NS, 0.5,0,SC_NS,true),
    rst("rst"),
    PE("PE"),
    src("src")
  {
    PE.clk(clk);
    PE.rst(rst);
    PE.act_in(act_in);
    PE.act_out(act_out);
    PE.weight_in(weight_in);
    PE.weight_out(weight_out);
    PE.accum_in(accum_in);
    PE.accum_out(accum_out);

    src.clk(clk);
    src.rst(rst);
    src.act_in(act_in);
    src.act_out(act_out);
    src.weight_in(weight_in);
    src.weight_out(weight_out);
    src.accum_in(accum_in);
    src.accum_out(accum_out);

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
    nvhls::set_random_seed();
    testbench my_testbench("my_testbench");
    sc_start();
    cout << "CMODEL PASS" << endl;
    return 0;
};