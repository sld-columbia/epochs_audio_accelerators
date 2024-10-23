#ifndef __Rotate_2_H__
#define __Rotate_2_H__

#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
//#include <nvhls_connections.h>
#include <mc_connections.h>
#include <nvhls_message.h>
#include <nvhls_types.h>
#include <nvhls_vector.h>


SC_MODULE(Rotate2)
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

  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_cos_2_alpha;
  #pragma hls_direct_input  
  sc_in<ac_int<32,true>>   cfg_sin_2_alpha;
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_cos_2_beta;
  #pragma hls_direct_input  
  sc_in<ac_int<32,true>>   cfg_sin_2_beta;
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_cos_2_gamma;
  #pragma hls_direct_input  
  sc_in<ac_int<32,true>>   cfg_sin_2_gamma;

  // Local translation is, R-0 S-1 T-2 U-3 V-4
  // Note that global translation is different 
  // enum BFormatChannels3D
  // {
  //     kW,
  //     kY, kZ, kX,
  //     kV, kT, kR, kS, kU,
  //     kQ, kO, kM, kK, kL, kN, kP,
  //     kNumOfBformatChannels3D
  // };

  Connections::In <nvhls::nv_scvector<ac_fixed<32,16,true>, 5>> rotate2_in;
  Connections::Out<nvhls::nv_scvector<ac_fixed<32,16,true>, 5>> rotate2_out;

  enum ChannelNames { kR, kS, kT, kU, kV};


  SC_HAS_PROCESS(Rotate2);
  Rotate2(sc_module_name name_) : sc_module(name_) {
    SC_THREAD (run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    rotate2_in.Reset();
    rotate2_out.Reset();
    nvhls::nv_scvector<ac_fixed<32,16,true>, 5> reg_0, reg_1, reg_2, reg_3;

    #pragma hls_pipeline_init_interval 1
    #pragma pipeline_stall_mode flush
    while(1) {
      wait();
      reg_0 = rotate2_in.Pop();

      ac_fixed<32,16,true> cos_alpha_fxp; cos_alpha_fxp.set_slc(0, cfg_cos_alpha.read());
      ac_fixed<32,16,true> sin_alpha_fxp; sin_alpha_fxp.set_slc(0, cfg_sin_alpha.read()); 
      ac_fixed<32,16,true>  cos_beta_fxp;  cos_beta_fxp.set_slc(0,  cfg_cos_beta.read()); 
      ac_fixed<32,16,true>  sin_beta_fxp;  sin_beta_fxp.set_slc(0,  cfg_sin_beta.read());  
      ac_fixed<32,16,true> cos_gamma_fxp; cos_gamma_fxp.set_slc(0, cfg_cos_gamma.read());
      ac_fixed<32,16,true> sin_gamma_fxp; sin_gamma_fxp.set_slc(0, cfg_sin_gamma.read());

      ac_fixed<32,16,true> cos_2_alpha_fxp; cos_2_alpha_fxp.set_slc(0, cfg_cos_2_alpha.read());
      ac_fixed<32,16,true> sin_2_alpha_fxp; sin_2_alpha_fxp.set_slc(0, cfg_sin_2_alpha.read()); 
      ac_fixed<32,16,true>  cos_2_beta_fxp;  cos_2_beta_fxp.set_slc(0,  cfg_cos_2_beta.read()); 
      ac_fixed<32,16,true>  sin_2_beta_fxp;  sin_2_beta_fxp.set_slc(0,  cfg_sin_2_beta.read());  
      ac_fixed<32,16,true> cos_2_gamma_fxp; cos_2_gamma_fxp.set_slc(0, cfg_cos_2_gamma.read());
      ac_fixed<32,16,true> sin_2_gamma_fxp; sin_2_gamma_fxp.set_slc(0, cfg_sin_2_gamma.read());

      // derived coefficient
      ac_fixed<32,16,true>  sin_beta_fxp_sq = sin_beta_fxp*sin_beta_fxp;
      const ac_fixed<32,16,true>  sqrt3 = 1.7320508;
      
      const ac_fixed<32,16,true>  fxp025 = 0.25;
      const ac_fixed<32,16,true>  fxp050 = 0.5;
      const ac_fixed<32,16,true>  fxp075 = 0.75;

      // stage 1
      reg_1[kV] = -reg_0[kU]*sin_2_alpha_fxp  + reg_0[kV]*cos_2_alpha_fxp;
      reg_1[kT] = -reg_0[kS]*sin_alpha_fxp    + reg_0[kT]*cos_alpha_fxp;
      reg_1[kR] =  reg_0[kR];
      reg_1[kS] =  reg_0[kS]*cos_alpha_fxp    + reg_0[kT]*sin_alpha_fxp;
      reg_1[kU] =  reg_0[kU]*cos_2_alpha_fxp  + reg_0[kV]*sin_2_alpha_fxp;

      // stage 2
      reg_2[kV] =  -sin_beta_fxp*reg_1[kT]    + cos_beta_fxp*reg_1[kV];
      reg_2[kT] =  -cos_beta_fxp*reg_1[kT]    + sin_beta_fxp*reg_1[kV];
      reg_2[kR] =   (fxp075 * cos_2_beta_fxp + fxp025       ) * reg_1[kR] 
                  + (fxp050 * sqrt3 * sin_beta_fxp_sq       ) * reg_1[kU]
                  + (sqrt3     * sin_beta_fxp * cos_beta_fxp) * reg_1[kS];
      reg_2[kS] =   cos_2_beta_fxp                            * reg_1[kS] 
                  - (sqrt3     * cos_beta_fxp * sin_beta_fxp) * reg_1[kR] 
                  + (            cos_beta_fxp * sin_beta_fxp) * reg_1[kU];
      reg_2[kU] =   (fxp025 * cos_2_beta_fxp + fxp075       ) * reg_1[kU]
                  - (            cos_beta_fxp * sin_beta_fxp) * reg_1[kS] 
                  + (fxp050 * sqrt3 * sin_beta_fxp_sq       ) * reg_1[kR];

      // stage 2
      //reg_2[kV] =  -sin_beta_fxp*reg_1[kT]    + cos_beta_fxp*reg_1[kV];
      //reg_2[kT] =  -cos_beta_fxp*reg_1[kT]    + sin_beta_fxp*reg_1[kV];
      //reg_2[kR] =   (0.75      * cos_2_beta_fxp + 0.25)       * reg_1[kR] 
      //            + (0.5*sqrt3 * sin_beta_fxp_sq       )      * reg_1[kU]
      //            + (sqrt3     * sin_beta_fxp * cos_beta_fxp) * reg_1[kS];
      //reg_2[kS] =   cos_2_beta_fxp                            * reg_1[kS] 
      //            - (sqrt3     * cos_beta_fxp * sin_beta_fxp) * reg_1[kR] 
      //            + (            cos_beta_fxp * sin_beta_fxp) * reg_1[kU];
      //reg_2[kU] =   (0.25*cos_2_beta_fxp + 0.75)   * reg_1[kU]
      //            - cos_beta_fxp * sin_beta_fxp    * reg_1[kS] 
      //            + 0.5 * Sqrt3  * sin_beta_fxp_sq * reg_1[kR];


      // stage 3
      reg_3[kV] = -reg_2[kU]*sin_2_gamma_fxp + reg_2[kV]*cos_2_gamma_fxp;
      reg_3[kT] = -reg_2[kS]*sin_gamma_fxp   + reg_2[kT]*cos_gamma_fxp;
      reg_3[kR] =  reg_2[kR];
      reg_3[kS]=   reg_2[kS]*cos_gamma_fxp   + reg_2[kT]*sin_gamma_fxp;
      reg_3[kU] =  reg_2[kU]*cos_2_gamma_fxp + reg_2[kV]*sin_2_gamma_fxp;


      rotate2_out.Push(reg_3);
    }
  }
};




#endif

