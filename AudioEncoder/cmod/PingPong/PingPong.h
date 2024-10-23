//#pragma once

#ifndef __PingPong_H__
#define __PingPong_H__

#include <ac_sysc_macros.h>
#include <ac_channel.h>
#include <ac_array.h>
//#include <ac_sysc_trace.h>
#include <mc_connections.h>
//#include "RAM_lib/RAM_1R1W.h"
//#include "mc_fifo.h"


SC_MODULE(PingPong)
{
  sc_in<bool> CCS_INIT_S1(clk);
  sc_in<bool> CCS_INIT_S1(rst_bar);

  Connections::In<ac_int<16> >  CCS_INIT_S1(in1);
  Connections::Out<ac_int<16> > CCS_INIT_S1(out1);

  SC_CTOR(PingPong) {
    SC_THREAD(thread1);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
    SC_THREAD(thread2);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

    //Instantiated memories must bind clocks
    //mem.CK(clk);
    //mem.clk(clk);
  }

  void thread1() {
    bool ping_pong = false;
    in1.Reset();
    sync1.reset_sync_out();
    wait();                                 // WAIT
#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      ac_array<ac_int<16>, 64> mem;
      for (int i=0; i < 40; i++) {
        //mem[i + (8 * ping_pong)] = in1.Pop();
        mem[i] = in1.Pop();
        wait(); 
      }
      ac_mem.write(mem);
      sync1.sync_out();
      //ping_pong = !ping_pong;
      wait();
    }
  }

  void thread2() {
    bool ping_pong = false;
    out1.Reset();
    sync1.reset_sync_in();
    wait();                                 // WAIT

#pragma hls_pipeline_init_interval 1
#pragma pipeline_stall_mode flush
    while (1) {
      sync1.sync_in();
      ac_array<ac_int<16>, 64> mem = ac_mem.read();
      for (int i=0; i < 40; i++) {
        //out1.Push(mem[i + (8 * ping_pong)]);
        out1.Push(mem[i]);
        wait(); 
      }
      //ping_pong = !ping_pong;
      wait();
    }
  }

  private:
  Connections::SyncChannel CCS_INIT_S1(sync1); // memory synchronization between threads
  //RAM_1R1W_model<>::mem<ac_int<16>,128> CCS_INIT_S1(mem);//Ping-pong shared memory

  ac_channel<ac_array<ac_int<16>,64> > ac_mem;
};


#endif
