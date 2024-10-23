#ifndef __SysPE_H__
#define __SysPE_H__

#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
#include <nvhls_connections.h>

SC_MODULE(SysPE)
{
  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;

  Connections::In<NVINT8>   weight_in;
  Connections::In<NVINT8>   act_in;
  Connections::In<NVINT32>  accum_in;

  
  Connections::Out<NVINT8>  weight_out;
  Connections::Out<NVINT8>  act_out;
  Connections::Out<NVINT32>  accum_out;

  SC_HAS_PROCESS(SysPE);
  SysPE(sc_module_name name_) : sc_module(name_) {
    SC_THREAD (run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }
  
  bool is_weight_in, is_act_in, is_accum_in;

  NVINT8 weight_reg, weight_out_reg;
  NVINT8 act_reg;
  NVINT32 accum_reg;

  void run() {
    weight_in.Reset();
    act_in.Reset();
    accum_in.Reset();
    weight_out.Reset();
    act_out.Reset();
    accum_out.Reset();

    
    is_weight_in = 0;
    is_act_in = 0;
    is_accum_in = 0;

    #pragma hls_pipeline_init_interval 1
    while(1) {
      wait();
    
      NVINT8 tmp;
      if (weight_in.PopNB(tmp)){
        is_weight_in = 1;
        weight_out_reg = weight_reg;
        weight_reg = tmp;
      }
      if (!is_act_in) {
        is_act_in = act_in.PopNB(act_reg);
      }
      if (!is_accum_in) {
        is_accum_in = accum_in.PopNB(accum_reg);
      }
    
      if (is_act_in && is_accum_in) {
        is_act_in = 0;
        is_accum_in = 0;

        NVINT32 accum_out_reg = act_reg*weight_reg + accum_reg;

        act_out.Push(act_reg);
        accum_out.Push(accum_out_reg);
      }
      if (is_weight_in == 1) {
        is_weight_in = 0;
        weight_out.Push(weight_out_reg);
      }

    }
  }
};

#endif

