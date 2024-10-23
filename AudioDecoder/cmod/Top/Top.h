#ifndef __Top_H__
#define __Top_H__

#include "../Control/Control.h"
#include "../InputBuffer/InputBuffer.h"
#include "../Rotate/Rotate.h"
#include "../OutputBuffer/OutputBuffer.h"

SC_MODULE(Top)
{

  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;

  // CSRs, use 32 entries for now
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_regs[32];
  
  // Configurations 
  // 00:      N/A
  // 01:      N/A
  // 02:      cfg_block_size
  // 03:      cfg_input_base
  // 04:      cfg_output_base
  // 05-07:   N/A
  // 08:      cfg_cos_alpha;
  // 09:      cfg_sin_alpha;  
  // 0A:      cfg_cos_beta;
  // 0B:      cfg_sin_beta;
  // 0C:      cfg_cos_gamma;
  // 0D:      cfg_sin_gamma;  
  // 0E:      cfg_cos_2_alpha;
  // 0F:      cfg_sin_2_alpha;
  // 10:      cfg_cos_2_beta;
  // 11:      cfg_sin_2_beta;
  // 12:      cfg_cos_2_gamma;
  // 13:      cfg_sin_2_gamma;
  // 14:      cfg_cos_3_alpha;
  // 15:      cfg_sin_3_alpha;
  // 16:      cfg_cos_3_beta;
  // 17:      cfg_sin_3_beta;
  // 18:      cfg_cos_3_gamma;
  // 19:      cfg_sin_3_gamma;

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
  // InputBuffer -> Rotate
  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>> rotate_in;
  // Rotate -> OutputBuffer
  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>> rotate_out;

  Control       ctr;
  InputBuffer   inb;
  Rotate        rot;
  OutputBuffer  out;


  SC_HAS_PROCESS(Top);
  Top(sc_module_name name_) : sc_module(name_),
    ctr("ctr"),
    inb("inb"),
    rot("rot"),
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
    inb.cfg_block_size(cfg_regs[2]);
    inb.cfg_input_base(cfg_regs[3]);
    inb.sync1(sync1);
    inb.dma_read_ctrl(dma_read_ctrl);
    inb.dma_read_chnl(dma_read_chnl);
    inb.rotate_in(rotate_in);

    rot.clk(clk);
    rot.rst(rst);
    rot.cfg_cos_alpha   (cfg_regs[8]);
    rot.cfg_sin_alpha   (cfg_regs[9]);  
    rot.cfg_cos_beta    (cfg_regs[10]);
    rot.cfg_sin_beta    (cfg_regs[11]);
    rot.cfg_cos_gamma   (cfg_regs[12]);
    rot.cfg_sin_gamma   (cfg_regs[13]);  
    rot.cfg_cos_2_alpha (cfg_regs[14]);
    rot.cfg_sin_2_alpha (cfg_regs[15]);
    rot.cfg_cos_2_beta  (cfg_regs[16]);
    rot.cfg_sin_2_beta  (cfg_regs[17]);
    rot.cfg_cos_2_gamma (cfg_regs[18]);
    rot.cfg_sin_2_gamma (cfg_regs[19]);
    rot.cfg_cos_3_alpha (cfg_regs[20]);
    rot.cfg_sin_3_alpha (cfg_regs[21]);
    rot.cfg_cos_3_beta  (cfg_regs[22]);
    rot.cfg_sin_3_beta  (cfg_regs[23]);
    rot.cfg_cos_3_gamma (cfg_regs[24]);
    rot.cfg_sin_3_gamma (cfg_regs[25]); // 0x19
    rot.rotate_in(rotate_in);
    rot.rotate_out(rotate_out);


    out.clk(clk);
    out.rst(rst);
    out.cfg_block_size(cfg_regs[2]);
    out.cfg_output_base(cfg_regs[4]);
    out.sync2(sync2);
    out.sync3(sync3);
    out.dma_write_ctrl(dma_write_ctrl);
    out.dma_write_chnl(dma_write_chnl);
    out.rotate_out(rotate_out);
  }
};

#endif