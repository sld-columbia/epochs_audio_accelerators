#ifndef __Puch_Pop_H__
#define __Puch_Pop_H__

#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
//#include <nvhls_connections.h>
#include <mc_connections.h>
#include <nvhls_message.h>


SC_MODULE(PushPop)
{
  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;

  //Connections::SyncIn  sync1;
  Connections::In<bool> sync1;
  Connections::In<ac_int<64,true>> data_in;
  Connections::Out<ac_int<64,true>> data_out;

  //ac_int<64,true> mem[8192];

  SC_HAS_PROCESS(PushPop);
  PushPop(sc_module_name name_) : sc_module(name_) {
    SC_THREAD (run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    //sync1.reset_sync_in();
    sync1.Reset();
    data_in.Reset();
    data_out.Reset();

    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while (1) {
      wait();
      // start, racing between sync and Push
      //sync1.sync_in();
      sync1.Pop();
      wait();
      
      for (int i = 0; i < 1024; i++) {
        ac_int<64,true> tmp = data_in.Pop();
        wait();
        data_out.Push(tmp);
        wait();
      }
    }
  }

};
#endif
