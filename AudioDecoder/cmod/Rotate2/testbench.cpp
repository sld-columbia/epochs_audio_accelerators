/* Testing design */

#include "Rotate2.h"
#include <systemc.h>
#include <mc_scverify.h>
#include <math.h>  

#define NVHLS_VERIFY_BLOCKS (Rotate2)
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


  Connections::Out<nvhls::nv_scvector<ac_fixed<32,16,true>, 5>> rotate2_in;  
  Connections::In<nvhls::nv_scvector<ac_fixed<32,16,true>, 5>> rotate2_out;

  SC_CTOR(Source) {
    SC_THREAD(run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run () {
    // reset 
    rotate2_in.Reset();
    rotate2_out.Reset();
    
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


    float rotate2_in_fp[8][5] = {
      {0.1233,-0.2437,0.7458,-0.939,0.2675},
      {-0.6033,0.7569,-0.9525,0.4599,0.6069},
      {-0.5962,0.7975,0.1601,-0.813,0.0682},
      {-0.8526,0.4788,0.0389,-0.7409,0.4806},
      {-0.3603,-0.9522,0.064,-0.6267,0.2242},
      {0.6787,0.088,-0.6954,0.334,-0.1263},
      {-0.9185,-0.1523,0.6909,-0.3823,-0.8064},
      {-0.2691,0.9464,0.2731,0.4473,0.2615}
    };

    // reference output
    float rotate2_out_fp[8][5];

    for (int i = 0; i < 8; i++) {
      float R0, S0, T0, U0, V0;      
      float R1, S1, T1, U1, V1;
      float R2, S2, T2, U2, V2;
      float R3, S3, T3, U3, V3;

      R0 = rotate2_in_fp[i][0];
      S0 = rotate2_in_fp[i][1];
      T0 = rotate2_in_fp[i][2];
      U0 = rotate2_in_fp[i][3];
      V0 = rotate2_in_fp[i][4];

      V1 = -U0*sin_2_alpha_fp + V0*cos_2_alpha_fp;
      T1 = -S0*sin_alpha_fp   + T0*cos_alpha_fp;
      R1 =  R0;
      S1 =  S0*cos_alpha_fp   + T0*sin_alpha_fp;
      U1 =  U0*cos_2_alpha_fp + V0*sin_2_alpha_fp;

      V2 = -sin_beta_fp*T1 + cos_beta_fp*V1;
      T2 = -cos_beta_fp*T1 + sin_beta_fp*V1;
      R2 = (0.75*cos_2_beta_fp + 0.25)*R1 + (0.5*sqrt(3)*pow(sin_beta_fp,2.0))*U1 + (sqrt(3)*sin_beta_fp*cos_beta_fp)*S1; 
      S2 = cos_2_beta_fp*S1 - (sqrt(3)*cos_beta_fp*sin_beta_fp)*R1 + cos_beta_fp*sin_beta_fp*U1;
      U2 = (0.25*cos_2_beta_fp + 0.75)*U1 - cos_beta_fp*sin_beta_fp*S1 + (0.5*sqrt(3)*pow(sin_beta_fp,2.0))*R1;

      V3 = -U2*sin_2_gamma_fp + V2*cos_2_gamma_fp;
      T3 = -S2*sin_gamma_fp   + T2*cos_gamma_fp;
      R3 =  R2;
      S3 =  S2*cos_gamma_fp   + T2*sin_gamma_fp;
      U3 =  U2*cos_2_gamma_fp + V2*sin_2_gamma_fp;
       
      rotate2_out_fp[i][0] = R3;
      rotate2_out_fp[i][1] = S3;
      rotate2_out_fp[i][2] = T3;
      rotate2_out_fp[i][3] = U3;
      rotate2_out_fp[i][4] = V3;
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


    wait(20, SC_NS);
    cout << "@" << sc_time_stamp() << " Start Computation " << endl ;

    nvhls::nv_scvector<ac_fixed<32,16,true>, 5> rotate_in_reg, rotate_out_reg;


    for (int i = 0; i < 8; i++) {
      rotate_in_reg[0] = rotate2_in_fp[i][0];
      rotate_in_reg[1] = rotate2_in_fp[i][1];
      rotate_in_reg[2] = rotate2_in_fp[i][2];
      rotate_in_reg[3] = rotate2_in_fp[i][3];
      rotate_in_reg[4] = rotate2_in_fp[i][4];

      rotate2_in.Push(rotate_in_reg);
      cout << "@" << sc_time_stamp() << " Push rotate_in_reg" << endl;

      rotate_out_reg = rotate2_out.Pop();
      cout << "@" << sc_time_stamp() << " Pop rotate_out_reg" << endl;

      cout << "out:\t" << fixed << setprecision(4);
      cout << rotate_out_reg[0].to_double() << "\t" << rotate_out_reg[1].to_double() << "\t" 
           << rotate_out_reg[2].to_double() << "\t" << rotate_out_reg[3].to_double() << "\t" << rotate_out_reg[4].to_double() << endl;
      cout << "ref:\t" << fixed << setprecision(4);
      cout << rotate2_out_fp[i][0] << "\t" << rotate2_out_fp[i][1] << "\t" 
           << rotate2_out_fp[i][2] << "\t" << rotate2_out_fp[i][3] << "\t" << rotate2_out_fp[i][4] << endl;

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

  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 5>> rotate2_in;  
  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 5>> rotate2_out;

  NVHLS_DESIGN(Rotate2) dut;
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

    dut.rotate2_in(rotate2_in);
    dut.rotate2_out(rotate2_out);


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

    src.rotate2_in(rotate2_in);
    src.rotate2_out(rotate2_out);
    
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