#ifndef __Top_H__
#define __Top_H__

#include "../Control/Control.h"
#include "../InputBuffer/InputBuffer.h"
#include "../Encoder/Encoder.h"
#include "../OutputBuffer/OutputBuffer.h"

SC_MODULE(Top)
{

  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;


  // CSRs, use 48 entries for now
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_regs[48];
  
  // Configurations 
  // 00:      N/A
  // 01:      cfg_src_num
  // 02:      cfg_block_size
  // 03:      cfg_input_base
  // 04:      cfg_output_base
  // 05-0F:   N/A
  // 10-1F:   cfg_src_coeff[0~A]
  // 20-2F:   cfg_chan_coeff[0~A]


  // start done
  sc_in<bool> acc_start;    // start signal
  sc_out<bool> acc_done;    // done signal 

  // DMA ports 
  Connections::Out<dma_info_t> dma_read_ctrl;
  Connections::In<ac_int<64,true>> dma_read_chnl;
  Connections::Out<dma_info_t> dma_write_ctrl;
  Connections::Out<ac_int<64,true>> dma_write_chnl;

  // Internal syncs
  Connections::Combinational<bool>  sync1;
  Connections::Combinational<bool>  sync2;
  Connections::Combinational<bool>  sync3;

  // Internal Data
  // InputBuffer -> Encoder
  Connections::Combinational<ac_int<16,true>> audio_in;
  // Encoder -> OutputBuffer
  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>> audio_out;
  
  Control       ctr;
  InputBuffer   inb;
  Encoder       enc;
  OutputBuffer  out;


  SC_HAS_PROCESS(Top);
  Top(sc_module_name name_) : sc_module(name_),
    ctr("ctr"),
    inb("inb"),
    enc("enc"),
    out("out")
  {

    // port connections
    ctr.clk(clk);
    ctr.rst(rst);
    ctr.acc_start(acc_start);
    ctr.acc_done(acc_done);
    ctr.sync1(sync1);
    ctr.sync2(sync2);
    ctr.sync3(sync3);


    inb.clk(clk);
    inb.rst(rst);
    inb.cfg_src_num(cfg_regs[1]);
    inb.cfg_block_size(cfg_regs[2]);
    inb.cfg_input_base(cfg_regs[3]);
    inb.sync1(sync1);
    inb.dma_read_ctrl(dma_read_ctrl);
    inb.dma_read_chnl(dma_read_chnl);
    inb.audio_in(audio_in);

    enc.clk(clk);
    enc.rst(rst);
    enc.cfg_src_num(cfg_regs[1]);
    for (int i = 0; i < 16; i++) { 
      enc.cfg_src_coeff[i](cfg_regs[i+16]);
      enc.cfg_chan_coeff[i](cfg_regs[i+32]);
    }
    enc.audio_in(audio_in);
    enc.audio_out(audio_out);

    out.clk(clk);
    out.rst(rst);
    out.cfg_block_size(cfg_regs[2]);
    out.cfg_output_base(cfg_regs[4]);
    out.sync2(sync2);
    out.sync3(sync3);
    out.dma_write_ctrl(dma_write_ctrl);
    out.dma_write_chnl(dma_write_chnl);
    out.audio_out(audio_out);
  }
};


#endif