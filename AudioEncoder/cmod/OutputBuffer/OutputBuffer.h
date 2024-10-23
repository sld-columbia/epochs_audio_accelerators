#ifndef __Output_Buffer_H__
#define __Output_Buffer_H__

#include "../include/common.h"

#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
//#include <nvhls_connections.h>
#include <mc_connections.h>
#include <nvhls_message.h>
#include <nvhls_types.h>
#include <nvhls_vector.h>

// use sc_out

SC_MODULE(OutputBuffer)
{
  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;

  // CSRs, channel number is hardcoded
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_block_size;
  // base index (matching ESP dma format, the size is determined src_num, block_size)
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_output_base; 


  // Handshakes (Control <-> OutputBuffer), start
  Connections::In<bool>   sync2;
  // Handshakes (OutputBuffer <-> Control), done
  Connections::Out<bool>  sync3;
  
  // ESP DMA ports
  Connections::Out<dma_info_t> dma_write_ctrl;
  Connections::Out<ac_int<64,true>> dma_write_chnl;

  // Internal Data
  Connections::In<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>> audio_out;

  // 128KB = 32b*2048*16src, 2048 block size for 16 sources -> map to SRAM
  ac_int<64,true> mem[16384];

  SC_HAS_PROCESS(OutputBuffer);
  OutputBuffer(sc_module_name name_) : sc_module(name_) {
    SC_THREAD (run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }


  // TODO, transpose dimension to [channel][sample] => DONE
  // Pop two samples and store 16 channel of 2 samples
  void run() {
//    sync1.reset_sync_in();
    sync2.Reset();
    sync3.Reset();

    dma_write_ctrl.Reset();
    dma_write_chnl.Reset();
    audio_out.Reset();

    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while (1) {
      wait();
      sync2.Pop();
      wait();
      
      // number of samples = block_size*src_num, and each sample is 16b
      ac_int<32,true> block_size = cfg_block_size.read();
      ac_int<32,false> length = (block_size>>1)*16;     // 16 channel hardcoded
      
      // 64b per memory entry, and 16*32b corresponds to 8 enteries 
//      for (int i = 0; i < (length >> 3); i++) {
      nvhls::nv_scvector<ac_fixed<32,16,true>, 16> audio_out_reg;

      for (int i = 0; i < (length >> 3); i++) {  
        audio_out_reg = audio_out.Pop();

        for (int j = 0; j < 8; j++) { // litle endian 
          ac_int<32,true> idx = (i << 3) + j;
          ac_int<64,true> data;
          //ac_int<32,true> data0; data0.set_slc(audio_out_reg[2*j]);
          //ac_int<32,true> data1; data1.set_slc(audio_out_reg[2*j]);

          data.set_slc(0, audio_out_reg[2*j].slc<32>(0));
          data.set_slc(32, audio_out_reg[2*j+1].slc<32>(0));
          mem[idx] = data;
          wait();
        }
      }

      // Push dma_write_ctrl
      dma_info_t dma_info_write;
      dma_info_write.index = cfg_output_base.read();  // global memory address
      dma_info_write.length = length;
      dma_info_write.size   = 3; // 64b mode
      dma_write_ctrl.Push(dma_info_write);

      // Push dma_write_data
      for (int i = 0; i < length; i++) {
        ac_int<64,true> data = mem[i];
        dma_write_chnl.Push(data);
        wait();
      }


      //bool tmp=1;
      sync3.Push(1); // done
    }
  }

};
#endif
