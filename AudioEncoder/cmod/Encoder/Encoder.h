#ifndef __Encoder_H__
#define __Encoder_H__

#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
//#include <nvhls_connections.h>
#include <mc_connections.h>
#include <nvhls_vector.h>


SC_MODULE(Encoder)
{
  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;

  // CSRs
  // Config number of source signals, 1 ~ 16 sound source (default = 16)
  // note that number of sources is configurable, but number of channel is fixed to 16
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_src_num;

  // Config source coefficient = combine amp scale and src_coeff in source code
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>  cfg_src_coeff[16];
  //sc_in<ac_fixed<32,16,true>>  cfg_src_coeff[16];

  // Config channel coefficient = combine internal/external gain and chan_coeff in source code
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>  cfg_chan_coeff[16];
  //  sc_in<ac_fixed<32,16,true>>  cfg_chan_coeff[16];

  // 16 bit wav format -32768 ~ + 32767
  Connections::In<ac_int<16,true>>   audio_in;  
  // 32 bit (fxp<bit=32,frac=16>) 16 channel output => 512 bit
  Connections::Out<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>> audio_out;
  

  SC_HAS_PROCESS(Encoder);
  Encoder(sc_module_name name_) : sc_module(name_) {
    SC_THREAD (run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }
  
  ac_int<8,true> src_idx;
  nvhls::nv_scvector<ac_fixed<32,16,true>, 16> audio_out_reg;

  void run() {
    audio_in.Reset();
    audio_out.Reset();

    src_idx = 0;
    
    #pragma hls_unroll yes
    for (int i = 0; i < 16; i++) { 
      audio_out_reg[i] = 0;
    } 

    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while(1) {
      wait();
      ac_int<16,true> audio_in_reg = audio_in.Pop();
      ac_fixed<32,16,true> src_coeff; src_coeff.set_slc(0, cfg_src_coeff[src_idx].read());

      // TODO: explore loop unrolling factors, 16x, 8x, 4x,
      #pragma hls_unroll yes
      for (int i = 0; i < 16; i++) {  // channel dim
        // normalize -32768 ~ + 32767 to (-1, 1) in fxp<32,16>
        ac_fixed<32,16,true> audio_tmp = audio_in_reg;  // copy software value
        //cout << "i = " << i << endl;
        //cout << "audio_tmp = " << audio_tmp << endl;
        audio_tmp = audio_tmp >> 15;                    // FIXME: neglect mul with 32768/32767
        //cout << "audio_tmp = " << audio_tmp  << endl;

        // encode
        ac_fixed<32,16,true> chan_coeff; chan_coeff.set_slc(0, cfg_chan_coeff[i].read());

        //cout << "src_coeff = " << src_coeff << endl;
        //cout << "chan_coeff = " << chan_coeff << endl;

        
        audio_tmp = audio_tmp * chan_coeff * src_coeff;   // 32b mul
        //cout << "audio_tmp = " << audio_tmp  << endl;
        audio_out_reg[i] = audio_out_reg[i] + audio_tmp;  // 32b add
        //cout << "audio_out_reg[i] = " << audio_out_reg[i]  << endl << endl;

      }

      // finish accumulate across src, push output and reset output reg 
      if (src_idx == cfg_src_num.read() - 1) {
        audio_out.Push(audio_out_reg);
        
        #pragma hls_unroll yes
        for (int i = 0; i < 16; i++) { 
          audio_out_reg[i] = 0;
        }
        src_idx = 0;
      } else {
        src_idx += 1;
      }
    }
  }
};

#endif

