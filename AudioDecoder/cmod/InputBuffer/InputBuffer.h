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
#include <nvhls_vector.h>

// use sc_out

SC_MODULE(InputBuffer)
{
  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;

  // CSRs
  //#pragma hls_direct_input
  //sc_in<ac_int<32,true>>   cfg_src_num;
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_block_size;
  // base index (matching ESP dma format, the size is determined src_num, block_size)
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_input_base; 


  Connections::In<bool>  sync1;

  // ESP DMA ports
  Connections::Out<dma_info_t> dma_read_ctrl;
  Connections::In<ac_int<64,true>> dma_read_chnl;
  // To function untis
  Connections::Out<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>>  rotate_in;

  ac_int<64,true> mem[16384];

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
    rotate_in.Reset();



    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while (1) {
      wait();
      sync1.Pop();
      wait();

      // number of samples = block_size*src_num, and each sample is 16b
      ac_int<32,true> block_size = cfg_block_size.read();
      //ac_int<32,true> src_num = cfg_src_num.read();
      // 32 bits per sample, and 16 channels
      ac_int<32,false> length = (block_size>>1)*16; 
      
      // Push dma_read_ctrl
      dma_info_t dma_info_read;
      dma_info_read.index = cfg_input_base.read();  // global memory address
      dma_info_read.length = length;
      dma_info_read.size   = 3; // 64b mode
      dma_read_ctrl.Push(dma_info_read);

      // Inside mem, we store input as data[src][sample/2][2], 
      // last dimension maps to a word
      // receive dma data and write mem
      for (int i = 0; i < length; i++) { 
        ac_int<64,true> data = dma_read_chnl.Pop();
        mem[i] = data;
        wait();
      }


      
      // read mem and push to output
      // for each sample
      for (int i = 0; i < block_size; i++) {
        
        nvhls::nv_scvector<ac_fixed<32,16,true>, 16> out_reg;
        // for each 2-chnnels
        for (int j = 0; j < 8; j++) {
          ac_int<32,true> idx = i*8 + j;
          ac_int<64,true> data = mem[idx]; 
          out_reg[2*j].set_slc<32>(0, data.slc<32>(0));
          out_reg[2*j + 1].set_slc<32>(0, data.slc<32>(32));
          wait();
        }
        rotate_in.Push(out_reg);
      }

    }
  }
};


#endif