/* Testing design */

#include "Rotate1.h"
#include <systemc.h>
#include <mc_scverify.h>

#define NVHLS_VERIFY_BLOCKS (Rotate1)
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

  Connections::Out<nvhls::nv_scvector<ac_fixed<32,16,true>, 3>> rotate1_in;  
  Connections::In<nvhls::nv_scvector<ac_fixed<32,16,true>, 3>> rotate1_out;

  SC_CTOR(Source) {
    SC_THREAD(run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run () {
    // reset 
    rotate1_in.Reset();
    rotate1_out.Reset();
    cfg_cos_alpha.write(0);
    cfg_sin_alpha.write(0);
    cfg_cos_beta.write(0);
    cfg_sin_beta.write(0);
    cfg_cos_gamma.write(0);
    cfg_sin_gamma.write(0);


    // compute reference input/output
    float cos_alpha_fp =  0.6014;
    float sin_alpha_fp =  0.9303;
    float cos_beta_fp  = -0.7021;
    float sin_beta_fp  = -0.8568;
    float cos_gamma_fp = -0.5455;
    float sin_gamma_fp = -0.3376;


    float rotate1_in_fp[8][3] = {
      { 0.7165, 0.3126,-0.1185},
      { 0.2234,-0.1045, 0.6031},
      { 0.4798, 0.9086,-0.3665},
      { 0.6029, 0.6937, 0.0274},
      {-0.1304, 0.0008, 0.2887},
      { 0.6452,-0.6981,-0.0855},
      { 0.5452,-0.9171, 0.2282},
      {-0.0092,-0.2698, 0.9726}
    };

    // reference output
    float rotate1_out_fp[8][3];


    for (int i = 0; i < 8; i++) {
      float x1, y1, z1;       
      float x2, y2, z2; 


      x1 =  rotate1_in_fp[i][0]*cos_alpha_fp + rotate1_in_fp[i][1]*sin_alpha_fp;
      y1 = -rotate1_in_fp[i][0]*sin_alpha_fp + rotate1_in_fp[i][1]*cos_alpha_fp;
      z1 = rotate1_in_fp[i][2];

      x2 = x1*cos_beta_fp + z1*sin_beta_fp;
      y2 = y1;
      z2 = x1*sin_beta_fp + z1*cos_beta_fp;

      //cout << x2 << "\t" << y2 << "\t" << z2 << endl;
      //cout << x2 << "\t" << cos_gamma_fp << "\t" << y2 << "\t" << sin_gamma_fp << endl;

      rotate1_out_fp[i][0] =  x2*cos_gamma_fp + y2*sin_gamma_fp;
      rotate1_out_fp[i][1] = -x2*sin_gamma_fp + y2*cos_gamma_fp;
      rotate1_out_fp[i][2] =  z2;
    }

    // Wait for initial reset
    wait(100, SC_NS);

    ac_fixed<32,16,true> cos_alpha_fixed = cos_alpha_fp;
    ac_fixed<32,16,true> sin_alpha_fixed = sin_alpha_fp;
    ac_fixed<32,16,true> cos_beta_fixed  = cos_beta_fp ;
    ac_fixed<32,16,true> sin_beta_fixed  = sin_beta_fp ;
    ac_fixed<32,16,true> cos_gamma_fixed = cos_gamma_fp;
    ac_fixed<32,16,true> sin_gamma_fixed = sin_gamma_fp;

    cfg_cos_alpha.write(cos_alpha_fixed.slc<32>(0));
    cfg_sin_alpha.write(sin_alpha_fixed.slc<32>(0));
    cfg_cos_beta .write( cos_beta_fixed.slc<32>(0));
    cfg_sin_beta .write( sin_beta_fixed.slc<32>(0));
    cfg_cos_gamma.write(cos_gamma_fixed.slc<32>(0));
    cfg_sin_gamma.write(sin_gamma_fixed.slc<32>(0));

    wait(20, SC_NS);
    cout << "@" << sc_time_stamp() << " Start Computation " << endl ;

    
    nvhls::nv_scvector<ac_fixed<32,16,true>, 3> rotate_in_reg, rotate_out_reg;

    for (int i = 0; i < 8; i++) {
      rotate_in_reg[0] = rotate1_in_fp[i][0];
      rotate_in_reg[1] = rotate1_in_fp[i][1];
      rotate_in_reg[2] = rotate1_in_fp[i][2];

      rotate1_in.Push(rotate_in_reg);
      cout << "@" << sc_time_stamp() << " Push rotate_in_reg" << endl;

      rotate_out_reg = rotate1_out.Pop();
      cout << "@" << sc_time_stamp() << " Pop rotate_out_reg" << endl;

      cout << "out:\t" << fixed << setprecision(4);
      cout << rotate_out_reg[0].to_double() << "\t" << rotate_out_reg[1].to_double() << "\t" << rotate_out_reg[2].to_double() << endl;
      cout << "ref:\t" << fixed << setprecision(4);
      cout << rotate1_out_fp[i][0] << "\t" << rotate1_out_fp[i][1] << "\t" << rotate1_out_fp[i][2] << endl;

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

  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 3>> rotate1_in;  
  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 3>> rotate1_out;

  NVHLS_DESIGN(Rotate1) dut;
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
    dut.cfg_cos_beta(cfg_cos_beta);
    dut.cfg_sin_beta(cfg_sin_beta);
    dut.cfg_cos_gamma(cfg_cos_gamma);
    dut.cfg_sin_gamma(cfg_sin_gamma);
    dut.rotate1_in(rotate1_in);
    dut.rotate1_out(rotate1_out);

    src.clk(clk);
    src.rst(rst);
    src.cfg_cos_alpha(cfg_cos_alpha);
    src.cfg_sin_alpha(cfg_sin_alpha);
    src.cfg_cos_beta(cfg_cos_beta);
    src.cfg_sin_beta(cfg_sin_beta);
    src.cfg_cos_gamma(cfg_cos_gamma);
    src.cfg_sin_gamma(cfg_sin_gamma);
    src.rotate1_in(rotate1_in);
    src.rotate1_out(rotate1_out);
    
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