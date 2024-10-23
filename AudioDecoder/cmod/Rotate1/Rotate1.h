#ifndef __Rotate_1_H__
#define __Rotate_1_H__


#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
//#include <nvhls_connections.h>
#include <mc_connections.h>
#include <nvhls_message.h>
#include <nvhls_types.h>
#include <nvhls_vector.h>



SC_MODULE(Rotate1)
{
  public:
  sc_in_clk     clk;
  sc_in<bool>   rst;

  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_cos_alpha;
  #pragma hls_direct_input  
  sc_in<ac_int<32,true>>   cfg_sin_alpha;
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_cos_beta;
  #pragma hls_direct_input  
  sc_in<ac_int<32,true>>   cfg_sin_beta;
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_cos_gamma;
  #pragma hls_direct_input  
  sc_in<ac_int<32,true>>   cfg_sin_gamma;



  // Local translation is, 0: kX, 1: kY, 2: kZ
  // Note that global translation is different 
  // enum BFormatChannels3D
  // {
  //     kW,
  //     kY, kZ, kX,
  //     kV, kT, kR, kS, kU,
  //     kQ, kO, kM, kK, kL, kN, kP,
  //     kNumOfBformatChannels3D
  // };

  Connections::In<nvhls::nv_scvector<ac_fixed<32,16,true>, 3>>  rotate1_in;
  Connections::Out<nvhls::nv_scvector<ac_fixed<32,16,true>, 3>> rotate1_out;


  enum ChannelNames { kX, kY, kZ};

  SC_HAS_PROCESS(Rotate1);
  Rotate1(sc_module_name name_) : sc_module(name_) {
    SC_THREAD (run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }



  void run() {
    rotate1_in.Reset();
    rotate1_out.Reset();

    nvhls::nv_scvector<ac_fixed<32,16,true>, 3> reg_0, reg_1, reg_2, reg_3;

    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while(1) {
      wait();
      
      reg_0 = rotate1_in.Pop();

      ac_fixed<32,16,true> cos_alpha_fxp; cos_alpha_fxp.set_slc(0, cfg_cos_alpha.read());
      ac_fixed<32,16,true> sin_alpha_fxp; sin_alpha_fxp.set_slc(0, cfg_sin_alpha.read()); 
      ac_fixed<32,16,true>  cos_beta_fxp;  cos_beta_fxp.set_slc(0,  cfg_cos_beta.read()); 
      ac_fixed<32,16,true>  sin_beta_fxp;  sin_beta_fxp.set_slc(0,  cfg_sin_beta.read());  
      ac_fixed<32,16,true> cos_gamma_fxp; cos_gamma_fxp.set_slc(0, cfg_cos_gamma.read());
      ac_fixed<32,16,true> sin_gamma_fxp; sin_gamma_fxp.set_slc(0, cfg_sin_gamma.read());


      // stage 1
      reg_1[kY] = -reg_0[kX]*sin_alpha_fxp + reg_0[kY]*cos_alpha_fxp;
      reg_1[kZ] =  reg_0[kZ];
      reg_1[kX] =  reg_0[kX]*cos_alpha_fxp + reg_0[kY]*sin_alpha_fxp;


      // TODO, need to double check the equation (there is some discrepency), use rotateOrder1_fxp for now
      // stage 2, 
      reg_2[kY] =  reg_1[kY];
      reg_2[kZ] =  reg_1[kX]*sin_beta_fxp + reg_1[kZ]*cos_beta_fxp;
      reg_2[kX] =  reg_1[kX]*cos_beta_fxp + reg_1[kZ]*sin_beta_fxp;

      //cout << reg_2[kX].to_double() << "\t" << reg_2[kY].to_double() << "\t" << reg_2[kZ].to_double() << endl;

      //cout << reg_2[kX].to_double() << "\t" << cos_gamma_fxp.to_double() << "\t" 
      //     << reg_2[kY].to_double() << "\t" << sin_gamma_fxp.to_double() << endl;


      // stage 3
      reg_3[kY] = -reg_2[kX]*sin_gamma_fxp + reg_2[kY]*cos_gamma_fxp;
      reg_3[kZ] =  reg_2[kZ];
      reg_3[kX] =  reg_2[kX]*cos_gamma_fxp + reg_2[kY]*sin_gamma_fxp;

      rotate1_out.Push(reg_3);
    }
  }
};

#endif
