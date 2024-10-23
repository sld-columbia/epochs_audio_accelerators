/* Testing design */

#include "Rotate3.h"
#include <systemc.h>
#include <mc_scverify.h>
#include <math.h>  

#define NVHLS_VERIFY_BLOCKS (Rotate3)
#include <nvhls_verify.h>
#include <testbench/nvhls_rand.h>
using namespace::std;

SC_MODULE (Source) {
  sc_in <bool> clk;
  sc_in <bool> rst;

  sc_out<ac_int<32,true>>   cfg_cos_alpha;
  sc_out<ac_int<32,true>>   cfg_sin_alpha;
  sc_out<ac_int<32,true>>   cfg_cos_beta;
  sc_out<ac_int<32,true>>   cfg_sin_beta;
  sc_out<ac_int<32,true>>   cfg_cos_gamma;
  sc_out<ac_int<32,true>>   cfg_sin_gamma;

  sc_out<ac_int<32,true>>   cfg_cos_2_alpha;
  sc_out<ac_int<32,true>>   cfg_sin_2_alpha;
  sc_out<ac_int<32,true>>   cfg_cos_2_beta;
  sc_out<ac_int<32,true>>   cfg_sin_2_beta;
  sc_out<ac_int<32,true>>   cfg_cos_2_gamma;
  sc_out<ac_int<32,true>>   cfg_sin_2_gamma;

  sc_out<ac_int<32,true>>   cfg_cos_3_alpha;
  sc_out<ac_int<32,true>>   cfg_sin_3_alpha;
  sc_out<ac_int<32,true>>   cfg_cos_3_beta;
  sc_out<ac_int<32,true>>   cfg_sin_3_beta;
  sc_out<ac_int<32,true>>   cfg_cos_3_gamma;
  sc_out<ac_int<32,true>>   cfg_sin_3_gamma;

  Connections::Out<nvhls::nv_scvector<ac_fixed<32,16,true>, 7>> rotate3_in;  
  Connections::In<nvhls::nv_scvector<ac_fixed<32,16,true>, 7>> rotate3_out;

  SC_CTOR(Source) {
    SC_THREAD(run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run () {
    // reset 
    rotate3_in.Reset();
    rotate3_out.Reset();
    
    cfg_cos_alpha.write(0);
    cfg_sin_alpha.write(0);
    cfg_cos_beta.write(0);
    cfg_sin_beta.write(0);
    cfg_cos_gamma.write(0);
    cfg_sin_gamma.write(0);

    cfg_cos_2_alpha.write(0);
    cfg_sin_2_alpha.write(0);
    cfg_cos_2_beta.write(0);
    cfg_sin_2_beta.write(0);
    cfg_cos_2_gamma.write(0);
    cfg_sin_2_gamma.write(0);

    cfg_cos_3_alpha.write(0);
    cfg_sin_3_alpha.write(0);
    cfg_cos_3_beta.write(0);
    cfg_sin_3_beta.write(0);
    cfg_cos_3_gamma.write(0);
    cfg_sin_3_gamma.write(0);

    // compute reference input/output
    float cos_alpha_fp =  0.6014;
    float sin_alpha_fp =  0.9303;
    float cos_beta_fp  = -0.7021;
    float sin_beta_fp  = -0.8568;
    float cos_gamma_fp = -0.5455;
    float sin_gamma_fp = -0.3376;

    float cos_2_alpha_fp = -0.6014;
    float sin_2_alpha_fp =  0.8703;
    float cos_2_beta_fp  = -0.2321;
    float sin_2_beta_fp  =  0.1568;
    float cos_2_gamma_fp =  0.4225;
    float sin_2_gamma_fp = -0.7432;

    float cos_3_alpha_fp = -0.6514;
    float sin_3_alpha_fp = -0.1723;
    float cos_3_beta_fp  =  0.7327;
    float sin_3_beta_fp  =  0.3528;
    float cos_3_gamma_fp = -0.8116;
    float sin_3_gamma_fp =  0.3491;

    float rotate3_in_fp[8][7] = {
      {0.0944,0.8519,0.0244,-0.2723,-0.7142,-0.3449,-0.7409},
      {0.8752,0.4599,-0.3191,0.0289,0.0883,0.0968,-0.5984},
      {-0.3208,-0.2138,-0.2631,-0.5668,0.3035,0.2201,-0.0404},
      {-0.1252,0.1913,-0.099,-0.5448,-0.4904,-0.03,0.0662},
      {0.2517,0.1744,-0.1558,-0.386,-0.8415,-0.2282,0.7797},
      {-0.6062,0.0431,0.8988,-0.2625,0.0251,-0.3952,-0.2543},
      {0.9722,0.0022,-0.4273,0.3096,0.8739,-0.1276,-0.0377},
      {0.6609,-0.0344,0.3632,0.0828,0.1386,0.3093,0.032}
    };
    
    float rotate3_out_fp[8][7];

    for (int i = 0; i < 8; i++) {
      float Q0, O0, M0, K0, L0, N0, P0;      
      float Q1, O1, M1, K1, L1, N1, P1;
      float Q2, O2, M2, K2, L2, N2, P2;
      float Q3, O3, M3, K3, L3, N3, P3;


      Q0 = rotate3_in_fp[i][0];
      O0 = rotate3_in_fp[i][1];
      M0 = rotate3_in_fp[i][2];
      K0 = rotate3_in_fp[i][3];
      L0 = rotate3_in_fp[i][4];
      N0 = rotate3_in_fp[i][5];
      P0 = rotate3_in_fp[i][6];

      Q1 = -P0 * sin_3_alpha_fp + Q0 * cos_3_alpha_fp;
      O1 = -N0 * sin_2_alpha_fp + O0 * cos_2_alpha_fp;
      M1 = -L0 * sin_alpha_fp   + M0 * cos_alpha_fp;
      K1 =  K0;
      L1 =  L0 * cos_alpha_fp   + M0 * sin_alpha_fp;
      N1 =  N0 * cos_2_alpha_fp + O0 * sin_2_alpha_fp;
      P1 =  P0 * cos_3_alpha_fp + Q0 * sin_3_alpha_fp;


      Q2 = 0.125f * Q1 * (5.f + 3.f*cos_2_beta_fp)
          - sqrt(1.5f) * O1 *cos_beta_fp * sin_beta_fp
          + 0.25f * sqrt(15) * M1 * pow(sin_beta_fp,2.0f);
      O2 = O1 * cos_2_beta_fp
          - sqrt(2.5f) * M1 * cos_beta_fp * sin_beta_fp
          + sqrt(1.5f) * Q1 * cos_beta_fp * sin_beta_fp;
      M2 = 0.125f * M1 * (3.f + 5.f*cos_2_beta_fp)
          - sqrt(2.5f) * O1 *cos_beta_fp * sin_beta_fp
          + 0.25f * sqrt(15) * Q1 * pow(sin_beta_fp,2.0f);
      K2 = 0.25f * K1 * cos_beta_fp * (-1.f + 15.f*cos_2_beta_fp)
          + 0.5f * sqrt(15) * N1 * cos_beta_fp * pow(sin_beta_fp,2.f)
          + 0.5f * sqrt(2.5f) * P1 * pow(sin_beta_fp,3.f)
          + 0.125f * sqrt(1.5f) * L1 * (sin_beta_fp + 5.f * sin_3_beta_fp);
      L2 = 0.0625f * L1 * (cos_beta_fp + 15.f * cos_3_beta_fp)
          + 0.25f * sqrt(2.5f) * N1 * (1.f + 3.f * cos_2_beta_fp) * sin_beta_fp
          + 0.25f * sqrt(15) *  P1 * cos_beta_fp * pow(sin_beta_fp,2.f)
          - 0.125 * sqrt(1.5f) * K1 * (sin_beta_fp + 5.f * sin_3_beta_fp);
      N2 = 0.125f * N1 * (5.f * cos_beta_fp + 3.f * cos_3_beta_fp)
          + 0.25f * sqrt(1.5f) * P1 * (3.f + cos_2_beta_fp) * sin_beta_fp
          + 0.5f * sqrt(15) *   K1 * cos_beta_fp * pow(sin_beta_fp,2.f)
          + 0.125 * sqrt(2.5f) * L1 * (sin_beta_fp - 3.f * sin_3_beta_fp);
      P2 = 0.0625f * P1 * (15.f * cos_beta_fp + cos_3_beta_fp)
          - 0.25f * sqrt(1.5f) * N1 * (3.f + cos_2_beta_fp) * sin_beta_fp
          + 0.25f * sqrt(15) *  L1 * cos_beta_fp * pow(sin_beta_fp,2.f)
          - 0.5 * sqrt(2.5f) *   K1 * pow(sin_beta_fp,3.f);
    
      Q3 = -P2 * sin_3_gamma_fp + Q2 * cos_3_gamma_fp;
      O3 = -N2 * sin_2_gamma_fp + O2 * cos_2_gamma_fp;
      M3 = -L2 * sin_gamma_fp   + M2 * cos_gamma_fp;
      K3 =  K2;
      L3 =  L2 * cos_gamma_fp   + M2 * sin_gamma_fp;
      N3 =  N2 * cos_2_gamma_fp + O2 * sin_2_gamma_fp;
      P3 =  P2 * cos_3_gamma_fp + Q2 * sin_3_gamma_fp;    
    
      rotate3_out_fp[i][0] = Q3;
      rotate3_out_fp[i][1] = O3;
      rotate3_out_fp[i][2] = M3;
      rotate3_out_fp[i][3] = K3;
      rotate3_out_fp[i][4] = L3;
      rotate3_out_fp[i][5] = N3;
      rotate3_out_fp[i][6] = P3;    
    }

    // Wait for initial reset
    wait(100, SC_NS);

    ac_fixed<32,16,true> cos_alpha_fixed = cos_alpha_fp;
    ac_fixed<32,16,true> sin_alpha_fixed = sin_alpha_fp;
    ac_fixed<32,16,true> cos_beta_fixed  = cos_beta_fp ;
    ac_fixed<32,16,true> sin_beta_fixed  = sin_beta_fp ;
    ac_fixed<32,16,true> cos_gamma_fixed = cos_gamma_fp;
    ac_fixed<32,16,true> sin_gamma_fixed = sin_gamma_fp;

    ac_fixed<32,16,true> cos_2_alpha_fixed = cos_2_alpha_fp;
    ac_fixed<32,16,true> sin_2_alpha_fixed = sin_2_alpha_fp;
    ac_fixed<32,16,true> cos_2_beta_fixed  = cos_2_beta_fp ;
    ac_fixed<32,16,true> sin_2_beta_fixed  = sin_2_beta_fp ;
    ac_fixed<32,16,true> cos_2_gamma_fixed = cos_2_gamma_fp;
    ac_fixed<32,16,true> sin_2_gamma_fixed = sin_2_gamma_fp;


    ac_fixed<32,16,true> cos_3_alpha_fixed = cos_3_alpha_fp;
    ac_fixed<32,16,true> sin_3_alpha_fixed = sin_3_alpha_fp;
    ac_fixed<32,16,true> cos_3_beta_fixed  = cos_3_beta_fp ;
    ac_fixed<32,16,true> sin_3_beta_fixed  = sin_3_beta_fp ;
    ac_fixed<32,16,true> cos_3_gamma_fixed = cos_3_gamma_fp;
    ac_fixed<32,16,true> sin_3_gamma_fixed = sin_3_gamma_fp;

    cfg_cos_alpha.write(cos_alpha_fixed.slc<32>(0));
    cfg_sin_alpha.write(sin_alpha_fixed.slc<32>(0));
    cfg_cos_beta .write( cos_beta_fixed.slc<32>(0));
    cfg_sin_beta .write( sin_beta_fixed.slc<32>(0));
    cfg_cos_gamma.write(cos_gamma_fixed.slc<32>(0));
    cfg_sin_gamma.write(sin_gamma_fixed.slc<32>(0));

    cfg_cos_2_alpha.write(cos_2_alpha_fixed.slc<32>(0));
    cfg_sin_2_alpha.write(sin_2_alpha_fixed.slc<32>(0));
    cfg_cos_2_beta .write(cos_2_beta_fixed.slc<32>(0));
    cfg_sin_2_beta .write(sin_2_beta_fixed.slc<32>(0));
    cfg_cos_2_gamma.write(cos_2_gamma_fixed.slc<32>(0));
    cfg_sin_2_gamma.write(sin_2_gamma_fixed.slc<32>(0));

    cfg_cos_3_alpha.write(cos_3_alpha_fixed.slc<32>(0));
    cfg_sin_3_alpha.write(sin_3_alpha_fixed.slc<32>(0));
    cfg_cos_3_beta .write(cos_3_beta_fixed.slc<32>(0));
    cfg_sin_3_beta .write(sin_3_beta_fixed.slc<32>(0));
    cfg_cos_3_gamma.write(cos_3_gamma_fixed.slc<32>(0));
    cfg_sin_3_gamma.write(sin_3_gamma_fixed.slc<32>(0));

    wait(20, SC_NS);
    cout << "@" << sc_time_stamp() << " Start Computation " << endl ;

    nvhls::nv_scvector<ac_fixed<32,16,true>, 7> rotate_in_reg, rotate_out_reg;

    for (int i = 0; i < 8; i++) {
      rotate_in_reg[0] = rotate3_in_fp[i][0];
      rotate_in_reg[1] = rotate3_in_fp[i][1];
      rotate_in_reg[2] = rotate3_in_fp[i][2];
      rotate_in_reg[3] = rotate3_in_fp[i][3];
      rotate_in_reg[4] = rotate3_in_fp[i][4];
      rotate_in_reg[5] = rotate3_in_fp[i][5];
      rotate_in_reg[6] = rotate3_in_fp[i][6];

      rotate3_in.Push(rotate_in_reg);
      cout << "@" << sc_time_stamp() << " Push rotate_in_reg" << endl;

      rotate_out_reg = rotate3_out.Pop();
      cout << "@" << sc_time_stamp() << " Pop rotate_out_reg" << endl;

      cout << "out:\t" << fixed << setprecision(4);
      cout << rotate_out_reg[0].to_double() << "\t" << rotate_out_reg[1].to_double() << "\t" 
           << rotate_out_reg[2].to_double() << "\t" << rotate_out_reg[3].to_double() << "\t" 
           << rotate_out_reg[4].to_double() << "\t" << rotate_out_reg[5].to_double() << "\t" 
           << rotate_out_reg[6].to_double() <<endl;
      cout << "ref:\t" << fixed << setprecision(4);
      cout << rotate3_out_fp[i][0] << "\t" << rotate3_out_fp[i][1] << "\t" 
           << rotate3_out_fp[i][2] << "\t" << rotate3_out_fp[i][3] << "\t" 
           << rotate3_out_fp[i][4] << "\t" << rotate3_out_fp[i][5] << "\t" 
           << rotate3_out_fp[i][6] <<endl;

      cout << endl;
    }

  }
};


SC_MODULE (testbench) {
  sc_clock clk;
  sc_signal<bool> rst;

  sc_signal<ac_int<32,true>>   cfg_cos_alpha;
  sc_signal<ac_int<32,true>>   cfg_sin_alpha;
  sc_signal<ac_int<32,true>>   cfg_cos_beta;
  sc_signal<ac_int<32,true>>   cfg_sin_beta;
  sc_signal<ac_int<32,true>>   cfg_cos_gamma;
  sc_signal<ac_int<32,true>>   cfg_sin_gamma;

  sc_signal<ac_int<32,true>>   cfg_cos_2_alpha;
  sc_signal<ac_int<32,true>>   cfg_sin_2_alpha;
  sc_signal<ac_int<32,true>>   cfg_cos_2_beta;
  sc_signal<ac_int<32,true>>   cfg_sin_2_beta;
  sc_signal<ac_int<32,true>>   cfg_cos_2_gamma;
  sc_signal<ac_int<32,true>>   cfg_sin_2_gamma;


  sc_signal<ac_int<32,true>>   cfg_cos_3_alpha;
  sc_signal<ac_int<32,true>>   cfg_sin_3_alpha;
  sc_signal<ac_int<32,true>>   cfg_cos_3_beta;
  sc_signal<ac_int<32,true>>   cfg_sin_3_beta;
  sc_signal<ac_int<32,true>>   cfg_cos_3_gamma;
  sc_signal<ac_int<32,true>>   cfg_sin_3_gamma;

  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 7>> rotate3_in;  
  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 7>> rotate3_out;

  NVHLS_DESIGN(Rotate3) dut;
  //Rotate1 dut;
  Source src;

  SC_HAS_PROCESS(testbench);
  testbench(sc_module_name name)
  : sc_module(name),
    clk("clk", 1, SC_NS, 0.5,0,SC_NS,true),
    rst("rst"),
    dut("dut"),
    src("src")
  {
    
    dut.clk(clk);
    dut.rst(rst);
    dut.cfg_cos_alpha(cfg_cos_alpha);
    dut.cfg_sin_alpha(cfg_sin_alpha);
    dut.cfg_cos_beta (cfg_cos_beta);
    dut.cfg_sin_beta (cfg_sin_beta);
    dut.cfg_cos_gamma(cfg_cos_gamma);
    dut.cfg_sin_gamma(cfg_sin_gamma);

    dut.cfg_cos_2_alpha(cfg_cos_2_alpha);
    dut.cfg_sin_2_alpha(cfg_sin_2_alpha);
    dut.cfg_cos_2_beta (cfg_cos_2_beta);
    dut.cfg_sin_2_beta (cfg_sin_2_beta);
    dut.cfg_cos_2_gamma(cfg_cos_2_gamma);
    dut.cfg_sin_2_gamma(cfg_sin_2_gamma);

    dut.cfg_cos_3_alpha(cfg_cos_3_alpha);
    dut.cfg_sin_3_alpha(cfg_sin_3_alpha);
    dut.cfg_cos_3_beta (cfg_cos_3_beta);
    dut.cfg_sin_3_beta (cfg_sin_3_beta);
    dut.cfg_cos_3_gamma(cfg_cos_3_gamma);
    dut.cfg_sin_3_gamma(cfg_sin_3_gamma);

    dut.rotate3_in(rotate3_in);
    dut.rotate3_out(rotate3_out);


    src.clk(clk);
    src.rst(rst);
    src.cfg_cos_alpha(cfg_cos_alpha);
    src.cfg_sin_alpha(cfg_sin_alpha);
    src.cfg_cos_beta(cfg_cos_beta);
    src.cfg_sin_beta(cfg_sin_beta);
    src.cfg_cos_gamma(cfg_cos_gamma);
    src.cfg_sin_gamma(cfg_sin_gamma);

    src.cfg_cos_2_alpha(cfg_cos_2_alpha);
    src.cfg_sin_2_alpha(cfg_sin_2_alpha);
    src.cfg_cos_2_beta (cfg_cos_2_beta);
    src.cfg_sin_2_beta (cfg_sin_2_beta);
    src.cfg_cos_2_gamma(cfg_cos_2_gamma);
    src.cfg_sin_2_gamma(cfg_sin_2_gamma);

    src.cfg_cos_3_alpha(cfg_cos_3_alpha);
    src.cfg_sin_3_alpha(cfg_sin_3_alpha);
    src.cfg_cos_3_beta (cfg_cos_3_beta);
    src.cfg_sin_3_beta (cfg_sin_3_beta);
    src.cfg_cos_3_gamma(cfg_cos_3_gamma);
    src.cfg_sin_3_gamma(cfg_sin_3_gamma);

    src.rotate3_in(rotate3_in);
    src.rotate3_out(rotate3_out);
    
    SC_THREAD(run);
  }

  void run() {
    rst = 1;
    wait(10, SC_NS);
    rst = 0;
    cout << "@" << sc_time_stamp() << " Asserting Reset " << endl ;
    wait(10, SC_NS);
    cout << "@" << sc_time_stamp() << " Deasserting Reset " << endl ;
    rst = 1;
    wait(10000,SC_NS);
    cout << "@" << sc_time_stamp() << " Stop " << endl ;
    sc_stop();
  }
};

int sc_main(int argc, char *argv[])
{
    //nvhls::set_random_seed();
    testbench my_testbench("my_testbench");
    sc_start();
    cout << "CMODEL PASS" << endl;
    return 0;
};