/* Testing design */

#include "Encoder.h"
#include <systemc.h>
#include <mc_scverify.h>

#define NVHLS_VERIFY_BLOCKS (Encoder)
#include <nvhls_verify.h>
#include <testbench/nvhls_rand.h>
using namespace::std;


SC_MODULE (Source) {
  sc_in <bool> clk;
  sc_in <bool> rst;

  sc_out<ac_int<32,true> >  cfg_src_num;
//  sc_out<ac_fixed<32,16,true>>  cfg_src_coeff[16];
  sc_out<ac_int<32,true>>  cfg_src_coeff[16];
//  sc_out<ac_fixed<32,16,true>>  cfg_chan_coeff[16];
  sc_out<ac_int<32,true>>  cfg_chan_coeff[16];

  Connections::Out<ac_int<16,true> >   audio_in;  
  Connections::In<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>> audio_out;

  SC_CTOR(Source) {
    SC_THREAD(run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    // generate synthetic values
    int cfg_src_num_int = 16;
    float cfg_src_coeff_fp[16] = {
      0.5938,1.0841,0.7713,0.9023,1.3416,1.2921,0.6968,1.4938,
      1.2288,0.9707,1.1912,0.7459,0.9162,1.3737,0.6934,0.7728
    };

    float cfg_chan_coeff_fp[16] = {
      1.3528,1.0193,1.1141,0.5576,0.6451,0.5893,1.2548,1.0511,
      1.0671,0.8487,0.6927,1.3675,0.9941,0.7267,0.6743,1.4581    
    };

    // test only 128 audio inputs (-32767, 32767), i.e. 8 samples from each source
    // audio_in_fp[sample][src]
    float audio_in_fp[8][16] = {
      {-10817,10253,-19607,22372,-11418,4086,222,-20823,-18070,16046,-2936,31292,-24151,24695,-5418,-940},
      {10966,-29296,-18308,788,-29502,24359,-8808,22710,11538,18944,-6344,-20097,-212,-27484,21841,19304},
      {-12084,25301,-10021,-25635,30548,-31592,9993,-928,-5927,14095,24909,360,21569,9148,-24430,13329},
      {10039,-16789,7911,19719,-21548,9563,-2524,8258,-8955,-13160,2056,19472,1601,29961,3994,-2049},
      {30813,-14230,14061,13067,20813,-30547,-14468,-27837,15913,-28020,-31262,32588,-13833,-11617,-7159,-1949},
      {-12348,-17301,-13249,-17834,-3633,-26922,-23731,-24694,-12060,11720,-18056,5054,-15816,14476,-29303,-3207},
      {17160,8348,3225,8915,3030,8009,12290,6957,30240,31077,21943,3723,-11993,23838,26628,-2445},
      {28411,4588,5457,14659,-25891,10527,11932,12366,-5238,-31400,23241,28830,6928,20758,-304,-9499}
    }; 

    // Reference output should be 8 samples from each channel
    // audio_in_int[sample][chan]
    float audio_out_fp[8][16];
    
    for (int t = 0; t < 8; t++) {       // sample
      for (int i = 0; i < 16; i++) {    // chan
        audio_out_fp[t][i] = 0;
        for (int j = 0; j < cfg_src_num_int; j++) {  // src
          audio_out_fp[t][i] += (audio_in_fp[t][j]/32768) * cfg_src_coeff_fp[j] * cfg_chan_coeff_fp[i];
          //audio_out_fp[t][i] =  audio_out_fp[t][i] ;
          //cout << audio_out_fp[t][i] << "\t";
        }
      }
    }

    // Reset
    cfg_src_num.write(0);
    for (int i = 0; i < 16; i++) { 
      cfg_src_coeff[i].write(0);
      cfg_chan_coeff[i].write(0);
    }

    audio_in.Reset();
    audio_out.Reset();

    // Wait for initial reset
    wait(100, SC_NS);

    // set configs
    ac_int<32,true> cfg_src_num_tmp = cfg_src_num_int;
    cfg_src_num.write(cfg_src_num_tmp);

    for (int i = 0; i < 16; i++) {
      ac_fixed<32,16,true> tmp0 = cfg_src_coeff_fp[i];
      cfg_src_coeff[i].write(tmp0.slc<32>(0) );       // explicit extraction of raw bits
      ac_fixed<32,16,true> tmp1 = cfg_chan_coeff_fp[i];
      cfg_chan_coeff[i].write(tmp1.slc<32>(0) );
    }
    wait(20, SC_NS);
    cout << "@" << sc_time_stamp() << " Start Computation " << endl ;
    // start computation 
    ac_int<16,true> audio_in_reg;
    nvhls::nv_scvector<ac_fixed<32,16,true>, 16> audio_out_reg;
    for (int t = 0; t < 8; t++) {       // sample
      for (int i = 0; i < cfg_src_num_int; i++) {  // src
        audio_in_reg = (int) audio_in_fp[t][i];
        audio_in.Push(audio_in_reg);
        cout << "@" << sc_time_stamp() << " Push audio_in_reg = " << audio_in_reg << endl;
        for(int j = 0; j < i; j++) {
          wait();
        }
      }
      // pop one output after aggregrate 16 sources in hardware
      wait();
      
      
      audio_out_reg = audio_out.Pop();
      cout << "@" << sc_time_stamp() << " Pop audio_out_reg" << endl;

      // Compare results across channels
      cout << "@" << sc_time_stamp() << "\t Sample " << t << endl;
      cout << "out:\t" << fixed << setprecision(4);
      for (int i = 0; i < 16; i++) {
        cout << (float) audio_out_reg[i].to_double() << "\t";
      }      
      cout << endl;
      cout << "ref:\t" << fixed << setprecision(4);
      for (int i = 0; i < 16; i++) {
        cout << audio_out_fp[t][i] << "\t";
      }
      cout << endl << endl;
    }


    wait(5); 
  }// void run()
};

SC_MODULE (testbench) {
  sc_clock clk;
  sc_signal<bool> rst;

  sc_signal<ac_int<32,true> >  cfg_src_num;
//  sc_signal<ac_fixed<32,16,true>>  cfg_src_coeff[16];
  sc_signal<ac_int<32,true>>  cfg_src_coeff[16];
// sc_signal<ac_fixed<32,16,true>>  cfg_chan_coeff[16];
  sc_signal<ac_int<32,true>>  cfg_chan_coeff[16];

  Connections::Combinational<ac_int<16,true> >   audio_in;  
  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>> audio_out;

  NVHLS_DESIGN(Encoder) dut;
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
    dut.cfg_src_num(cfg_src_num);
    for (int i = 0; i < 16; i++) { 
      dut.cfg_src_coeff[i](cfg_src_coeff[i]);
      dut.cfg_chan_coeff[i](cfg_chan_coeff[i]);
    } 

    dut.audio_in(audio_in);
    dut.audio_out(audio_out);


    src.clk(clk);
    src.rst(rst);
    src.cfg_src_num(cfg_src_num);
    for (int i = 0; i < 16; i++) { 
      src.cfg_src_coeff[i](cfg_src_coeff[i]);
      src.cfg_chan_coeff[i](cfg_chan_coeff[i]);
    } 

    src.audio_in(audio_in);
    src.audio_out(audio_out);
    
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