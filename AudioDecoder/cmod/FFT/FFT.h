#ifndef __FFT_H__
#define __FFT_H__

// FFT block with 32bit fixed point <32,16> datatype 

#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
//#include <nvhls_connections.h>
#include <mc_connections.h>
#include <nvhls_message.h>
#include <nvhls_vector.h>

SC_MODULE(FFT)
{
  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;

  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_block_size;

  Connections::In<bool>   sync4;
  Connections::Out<bool>  sync5;

  Connections::In<ac_fixed<32,16,true>>  fft_in;
  Connections::Out<ac_fixed<32,16,true>>  fft_out;


  //
  ac_int<64,true> mem[4096];





};

#endif