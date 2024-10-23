#ifndef __Input_Buffer_H__
#define __Input_Buffer_H__

#include "../include/common.h"

#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
//#include <nvhls_connections.h>
#include <mc_connections.h>
#include <nvhls_message.h>

// use sc_out

SC_MODULE(InputBuffer)
{
  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;

  // CSRs
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_src_num;
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_block_size;
  // base index (matching ESP dma format, the size is determined src_num, block_size)
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_input_base; 


  // Handshakes (Control <-> InputBuffer)
//  Connections::SyncIn  sync1;
  Connections::In<bool>  sync1;

  // ESP DMA ports
  Connections::Out<dma_info_t> dma_read_ctrl;
  Connections::In<ac_int<64,true>> dma_read_chnl;
  // To function untis
  Connections::Out<ac_int<16,true>> audio_in;

  // 64KB = 16b*2048*16src, 2048 block size for 16 sources -> map to SRAM
  ac_int<64,true> mem[8192];
  ac_int<64,true> mem_tmp[16];  // temperary cache 16 entries at most

  SC_HAS_PROCESS(InputBuffer);
  InputBuffer(sc_module_name name_) : sc_module(name_) {
    SC_THREAD (run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
//    sync1.reset_sync_in();
    sync1.Reset();
    dma_read_ctrl.Reset();
    dma_read_chnl.Reset();
    audio_in.Reset();

    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while (1) {
      wait();
      // start, racing between sync and Push
//      sync1.sync_in();
      sync1.Pop();
      wait();
      
      // number of samples = block_size*src_num, and each sample is 16b
      ac_int<32,true> block_size = cfg_block_size.read();
      ac_int<32,true> src_num = cfg_src_num.read();
      ac_int<32,false> length = (block_size>>2)*src_num;
      
      // Push dma_read_ctrl
      dma_info_t dma_info_read;
      dma_info_read.index = cfg_input_base.read();  // global memory address
      dma_info_read.length = length;
      dma_info_read.size   = 3; // 64b mode
      dma_read_ctrl.Push(dma_info_read);
      
      // Inside mem, we store input as data[src][sample/4][4]
      // receive dma data and write mem
      for (int i = 0; i < length; i++) { 
        ac_int<64,true> data = dma_read_chnl.Pop();
        mem[i] = data;
        wait();
      }


      // read mem (transpose) and push to output
      // read 4 samples per channel
      for (int i = 0; i < (block_size >> 2); i++) {
        for (int j = 0; j < src_num; j++) {
          ac_int<32,true> idx = j*(block_size >> 2);
          idx = idx + i;
          // fetch 4 samples of source j
          ac_int<64,true> data = mem[idx]; 
          mem_tmp[j] = data;
          wait();
        }

        // push output
        for (int k = 0; k < 4; k++) {  // for each 16b of mem_tmp[j]
          for (int j = 0; j < src_num; j++) {
            ac_int<16,true> out_reg = mem_tmp[j].slc<16>(16*k);
            audio_in.Push(out_reg);
            wait();
          }
        }
      }



    }
  }

};
#endif
