#ifndef __Rotate_H__
#define __Rotate_H__


#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
//#include <nvhls_connections.h>
#include <mc_connections.h>
#include <nvhls_message.h>
#include <nvhls_types.h>
#include <nvhls_vector.h>

SC_MODULE(Rotate)
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

  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_cos_3_alpha;
  #pragma hls_direct_input  
  sc_in<ac_int<32,true>>   cfg_sin_3_alpha;
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_cos_3_beta;
  #pragma hls_direct_input  
  sc_in<ac_int<32,true>>   cfg_sin_3_beta;
  #pragma hls_direct_input
  sc_in<ac_int<32,true>>   cfg_cos_3_gamma;
  #pragma hls_direct_input  
  sc_in<ac_int<32,true>>   cfg_sin_3_gamma;

  // We use this enum order as global translation of 
  // channel name to index, note that kW does not involve any computation
  // enum BFormatChannels3D
  // {
  //     kW,
  //     kY, kZ, kX,
  //     kV, kT, kR, kS, kU,
  //     kQ, kO, kM, kK, kL, kN, kP,
  //     kNumOfBformatChannels3D
  // };


  Connections::In<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>>  rotate_in;
  Connections::Out<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>> rotate_out;
  
  enum ChannelNames
  {
      kW,
      kY, kZ, kX,
      kV, kT, kR, kS, kU,
      kQ, kO, kM, kK, kL, kN, kP
  };

  SC_HAS_PROCESS(Rotate);
  Rotate(sc_module_name name_) : sc_module(name_) {
    SC_THREAD (run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    rotate_in.Reset();
    rotate_out.Reset();
    nvhls::nv_scvector<ac_fixed<32,16,true>, 16> reg_0, reg_1, reg_2, reg_3;
    
    ac_fixed<32,16,true> cos_alpha_fxp;  
    ac_fixed<32,16,true> sin_alpha_fxp;  
    ac_fixed<32,16,true>  cos_beta_fxp;  
    ac_fixed<32,16,true>  sin_beta_fxp;  
    ac_fixed<32,16,true> cos_gamma_fxp;  
    ac_fixed<32,16,true> sin_gamma_fxp;  
    ac_fixed<32,16,true> cos_2_alpha_fxp;
    ac_fixed<32,16,true> sin_2_alpha_fxp;
    ac_fixed<32,16,true> cos_2_beta_fxp; 
    ac_fixed<32,16,true> sin_2_beta_fxp; 
    ac_fixed<32,16,true> cos_2_gamma_fxp;
    ac_fixed<32,16,true> sin_2_gamma_fxp;
    ac_fixed<32,16,true> cos_3_alpha_fxp;
    ac_fixed<32,16,true> sin_3_alpha_fxp;
    ac_fixed<32,16,true> cos_3_beta_fxp; 
    ac_fixed<32,16,true> sin_3_beta_fxp; 
    ac_fixed<32,16,true> cos_3_gamma_fxp;
    ac_fixed<32,16,true> sin_3_gamma_fxp;
    // derived coefficient
    //ac_fixed<32,16,true>  sin_beta_fxp_sq;   
    //ac_fixed<32,16,true>  sin_beta_fxp_cb;

    const ac_fixed<32,16,true>  sqrt3   = 1.7320508;  //sqrt(3)
    const ac_fixed<32,16,true>  sqrt3_2 = 1.2247448;  //sqrt(1.5) 
    const ac_fixed<32,16,true>  sqrt15  = 3.8729833;  //sqrt(15)
    const ac_fixed<32,16,true>  sqrt5_2 = 1.5811388;  //sqrt(2.5)

    const ac_fixed<32,16,true>  fxp00625 = 0.0625;
    const ac_fixed<32,16,true>  fxp0125  =  0.125;
    const ac_fixed<32,16,true>  fxp025   =   0.25;
    const ac_fixed<32,16,true>  fxp050   =    0.5;
    const ac_fixed<32,16,true>  fxp075   =   0.75;

    // 16*32b per 4 cycles => 16B per 1 cycle 
    #pragma hls_pipeline_init_interval 4
    #pragma pipeline_stall_mode flush
    while(1) {
      reg_0 = rotate_in.Pop();

      cos_alpha_fxp.set_slc(0, cfg_cos_alpha.read());
      sin_alpha_fxp.set_slc(0, cfg_sin_alpha.read()); 
      cos_beta_fxp.set_slc(0,  cfg_cos_beta.read()); 
      sin_beta_fxp.set_slc(0,  cfg_sin_beta.read());  
      cos_gamma_fxp.set_slc(0, cfg_cos_gamma.read());
      sin_gamma_fxp.set_slc(0, cfg_sin_gamma.read());

      cos_2_alpha_fxp.set_slc(0, cfg_cos_2_alpha.read());
      sin_2_alpha_fxp.set_slc(0, cfg_sin_2_alpha.read()); 
      cos_2_beta_fxp.set_slc(0,  cfg_cos_2_beta.read()); 
      sin_2_beta_fxp.set_slc(0,  cfg_sin_2_beta.read());  
      cos_2_gamma_fxp.set_slc(0, cfg_cos_2_gamma.read());
      sin_2_gamma_fxp.set_slc(0, cfg_sin_2_gamma.read());

      cos_3_alpha_fxp.set_slc(0, cfg_cos_3_alpha.read());
      sin_3_alpha_fxp.set_slc(0, cfg_sin_3_alpha.read()); 
      cos_3_beta_fxp.set_slc(0,  cfg_cos_3_beta.read()); 
      sin_3_beta_fxp.set_slc(0,  cfg_sin_3_beta.read());  
      cos_3_gamma_fxp.set_slc(0, cfg_cos_3_gamma.read());
      sin_3_gamma_fxp.set_slc(0, cfg_sin_3_gamma.read());


      // derived coefficient
      //sin_beta_fxp_sq = sin_beta_fxp*sin_beta_fxp;   
      //sin_beta_fxp_cb = sin_beta_fxp*sin_beta_fxp*sin_beta_fxp;
      wait();

      //---- ROTATE 1 ---- 
      // stage 1
      reg_1[kY] = -reg_0[kX]*sin_alpha_fxp + reg_0[kY]*cos_alpha_fxp;
      reg_1[kZ] =  reg_0[kZ];
      reg_1[kX] =  reg_0[kX]*cos_alpha_fxp + reg_0[kY]*sin_alpha_fxp;
      
      
      // TODO, need to double check the equation (there is some discrepency), use rotateOrder1_fxp for now
      // stage 2, 
      reg_2[kY] =  reg_1[kY];
      reg_2[kZ] =  reg_1[kX]*sin_beta_fxp + reg_1[kZ]*cos_beta_fxp;
      reg_2[kX] =  reg_1[kX]*cos_beta_fxp + reg_1[kZ]*sin_beta_fxp;

      //cout << reg_2[kY]  << "\t" << reg_2[kZ] << "\t" <<  reg_2[kX]  << endl; 

      // stage 3
      reg_3[kY] = -reg_2[kX]*sin_gamma_fxp + reg_2[kY]*cos_gamma_fxp;
      reg_3[kZ] =  reg_2[kZ];
      reg_3[kX] =  reg_2[kX]*cos_gamma_fxp + reg_2[kY]*sin_gamma_fxp;

      //---- ROTATE 2 ---- 
      // stage 1
      reg_1[kV] = -reg_0[kU]*sin_2_alpha_fxp  + reg_0[kV]*cos_2_alpha_fxp;
      reg_1[kT] = -reg_0[kS]*sin_alpha_fxp    + reg_0[kT]*cos_alpha_fxp;
      reg_1[kR] =  reg_0[kR];
      reg_1[kS] =  reg_0[kS]*cos_alpha_fxp    + reg_0[kT]*sin_alpha_fxp;
      reg_1[kU] =  reg_0[kU]*cos_2_alpha_fxp  + reg_0[kV]*sin_2_alpha_fxp;

      //cout << reg_1[kV] << "\t" << reg_1[kT] << "\t" << reg_1[kR] << "\t" << reg_1[kS] << "\t" << reg_1[kU] << endl; 

      // stage 2
      reg_2[kV] =  -sin_beta_fxp*reg_1[kT]    + cos_beta_fxp*reg_1[kV];
      reg_2[kT] =  -cos_beta_fxp*reg_1[kT]    + sin_beta_fxp*reg_1[kV];
      reg_2[kR] =   (fxp075 * cos_2_beta_fxp + fxp025       ) * reg_1[kR] 
                  + (fxp050 * sqrt3 * sin_beta_fxp*sin_beta_fxp       ) * reg_1[kU]
                  + (sqrt3     * sin_beta_fxp * cos_beta_fxp) * reg_1[kS];
      reg_2[kS] =   cos_2_beta_fxp                            * reg_1[kS] 
                  - (sqrt3     * cos_beta_fxp * sin_beta_fxp) * reg_1[kR] 
                  + (            cos_beta_fxp * sin_beta_fxp) * reg_1[kU];
      reg_2[kU] =   (fxp025 * cos_2_beta_fxp + fxp075       ) * reg_1[kU]
                  - (            cos_beta_fxp * sin_beta_fxp) * reg_1[kS] 
                  + (fxp050 * sqrt3 * sin_beta_fxp*sin_beta_fxp       ) * reg_1[kR];

      // stage 3
      reg_3[kV] = -reg_2[kU]*sin_2_gamma_fxp + reg_2[kV]*cos_2_gamma_fxp;
      reg_3[kT] = -reg_2[kS]*sin_gamma_fxp   + reg_2[kT]*cos_gamma_fxp;
      reg_3[kR] =  reg_2[kR];
      reg_3[kS]=   reg_2[kS]*cos_gamma_fxp   + reg_2[kT]*sin_gamma_fxp;
      reg_3[kU] =  reg_2[kU]*cos_2_gamma_fxp + reg_2[kV]*sin_2_gamma_fxp;

      //---- ROTATE 3 ---- 
      // stage 1
      reg_1[kQ] = -reg_0[kP] * sin_3_alpha_fxp + reg_0[kQ] * cos_3_alpha_fxp;
      reg_1[kO] = -reg_0[kN] * sin_2_alpha_fxp + reg_0[kO] * cos_2_alpha_fxp;
      reg_1[kM] = -reg_0[kL] * sin_alpha_fxp   + reg_0[kM] * cos_alpha_fxp;
      reg_1[kK] =  reg_0[kK];
      reg_1[kL] =  reg_0[kL] * cos_alpha_fxp   + reg_0[kM] * sin_alpha_fxp;
      reg_1[kN] =  reg_0[kN] * cos_2_alpha_fxp + reg_0[kO] * sin_2_alpha_fxp;
      reg_1[kP] =  reg_0[kP] * cos_3_alpha_fxp + reg_0[kQ] * sin_3_alpha_fxp;

      // stage 2
      reg_2[kQ] =   fxp0125 * reg_1[kQ] * (5 + 3 * cos_2_beta_fxp) 
                  - sqrt3_2 * reg_1[kO] * cos_beta_fxp * sin_beta_fxp 
                  + fxp025  * sqrt15    * reg_1[kM]    * sin_beta_fxp*sin_beta_fxp;
      reg_2[kO] =             reg_1[kO] * cos_2_beta_fxp 
                  - sqrt5_2 * reg_1[kM] * cos_beta_fxp * sin_beta_fxp 
                  + sqrt3_2 * reg_1[kQ] * cos_beta_fxp * sin_beta_fxp;
      reg_2[kM] =   fxp0125 * reg_1[kM] * (3 + 5 * cos_2_beta_fxp) 
                  - sqrt5_2 * reg_1[kO] * cos_beta_fxp * sin_beta_fxp 
                  + fxp025  * sqrt15    * reg_1[kQ]    * sin_beta_fxp*sin_beta_fxp;
      reg_2[kK] =   fxp025  * reg_1[kK] * cos_beta_fxp * (-1 + 15 * cos_2_beta_fxp)
                  + fxp050  * sqrt15    * reg_1[kN]    * cos_beta_fxp * sin_beta_fxp*sin_beta_fxp
                  + fxp050  * sqrt5_2   * reg_1[kP]    * sin_beta_fxp*sin_beta_fxp*sin_beta_fxp 
                  + fxp0125 * sqrt3_2   * reg_1[kL]    * (sin_beta_fxp + 5 * sin_3_beta_fxp);
      reg_2[kL] =   fxp00625* reg_1[kL] * (cos_beta_fxp + 15 * cos_3_beta_fxp) 
                  + fxp025  * sqrt5_2   * reg_1[kN]    * (1 + 3 * cos_2_beta_fxp) * sin_beta_fxp 
                  + fxp025  * sqrt15    * reg_1[kP]    * cos_beta_fxp * sin_beta_fxp*sin_beta_fxp 
                  - fxp0125 * sqrt3_2   * reg_1[kK]    * (sin_beta_fxp + 5 * sin_3_beta_fxp);
      reg_2[kN] =   fxp0125 * reg_1[kN] * (5 * cos_beta_fxp + 3 * cos_3_beta_fxp) 
                  + fxp025  * sqrt3_2   * reg_1[kP]    * (3 + cos_2_beta_fxp) * sin_beta_fxp 
                  + fxp050  * sqrt15    * reg_1[kK]    * cos_beta_fxp * sin_beta_fxp*sin_beta_fxp
                  + fxp0125 * sqrt5_2   * reg_1[kL]    * (sin_beta_fxp - 3 * sin_3_beta_fxp);
      reg_2[kP] =   fxp00625* reg_1[kP] * (15 * cos_beta_fxp + cos_3_beta_fxp) 
                  - fxp025  * sqrt3_2   * reg_1[kN]    * (3 + cos_2_beta_fxp) * sin_beta_fxp 
                  + fxp025  * sqrt15    * reg_1[kL]    * cos_beta_fxp * sin_beta_fxp*sin_beta_fxp 
                  - fxp050  * sqrt5_2   * reg_1[kK]    * sin_beta_fxp*sin_beta_fxp*sin_beta_fxp;

      // stage 3
      reg_3[kQ] = -reg_2[kP] * sin_3_gamma_fxp + reg_2[kQ] * cos_3_gamma_fxp;
      reg_3[kO] = -reg_2[kN] * sin_2_gamma_fxp + reg_2[kO] * cos_2_gamma_fxp;
      reg_3[kM] = -reg_2[kL] * sin_gamma_fxp   + reg_2[kM] * cos_gamma_fxp;
      reg_3[kK] =  reg_2[kK];
      reg_3[kL] =  reg_2[kL] * cos_gamma_fxp   + reg_2[kM] * sin_gamma_fxp;
      reg_3[kN] =  reg_2[kN] * cos_2_gamma_fxp + reg_2[kO] * sin_2_gamma_fxp;
      reg_3[kP] =  reg_2[kP] * cos_3_gamma_fxp + reg_2[kQ] * sin_3_gamma_fxp;

      rotate_out.Push(reg_3);
    }
  }


};

#endif
