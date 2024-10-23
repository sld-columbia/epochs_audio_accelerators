#ifndef __Control_H__
#define __Control_H__


#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
//#include <nvhls_connections.h>
#include <mc_connections.h>
#include <nvhls_message.h>


SC_MODULE(Control)
{
  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;

  #pragma hls_direct_input
  sc_in<bool> acc_start;    // start signal
  sc_out<bool> acc_done;    // done signal 

  // Handshakes (Control <-> InputBuffer), start
  Connections::Out<bool>    sync1;
  // Handshakes (Control <-> OutputBuffer), start
  Connections::Out<bool>    sync2;
  // Handshakes (OutputBuffer <-> Control), done
  Connections::In<bool>     sync3;

  SC_HAS_PROCESS(Control);
  Control(sc_module_name name_) : sc_module(name_) {
    SC_THREAD (run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    acc_done.write(0);    
    sync1.Reset();
    sync2.Reset();
    sync3.Reset();


    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while(1) {
      wait();
      
      
      while (acc_start.read() == 0) {
        wait();
      }

      // delay start signal for 10 cycles
      for (int i = 0; i < 10; i++) {
        wait();
      }
      
      // start 
      sync1.Push(1);
      wait();
      sync2.Push(1);
      wait();

      // wait for complete
      sync3.Pop();

      // delay start signal for 10 cycles
      for (int i = 0; i < 10; i++) {
        wait();
      }
      

      // write done signal for 10s
      acc_done.write(1);

      // delay start signal for 10 cycles
      for (int i = 0; i < 10; i++) {
        wait();
      }
      acc_done.write(0);
    }


  }
};

#endif
