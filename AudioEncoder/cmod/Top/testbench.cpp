/* Testing design */

#include "Top.h"
#include <systemc.h>
#include <mc_scverify.h>

#define NVHLS_VERIFY_BLOCKS (Top)
#include <nvhls_verify.h>
#include <testbench/nvhls_rand.h>
using namespace::std;


SC_MODULE (Source) {
  sc_in <bool> clk;
  sc_in <bool> rst;

  sc_out<ac_int<32,true>>   cfg_regs[48];  
  // Configurations 
  // 00:      N/A
  // 01:      cfg_src_num
  // 02:      cfg_block_size
  // 03:      cfg_input_base
  // 04:      cfg_output_base
  // 05-0F:   N/A

  // 10-1F:   cfg_src_coeff[0~F]
  // 20-2F:   cfg_chan_coeff[0~F]

  sc_out<bool> acc_start;    // start signal
  sc_in<bool> acc_done;    // done signal 

  // DMA ports 
  Connections::In<dma_info_t> dma_read_ctrl;
  Connections::Out<ac_int<64,true>> dma_read_chnl;
  Connections::In<dma_info_t> dma_write_ctrl;
  Connections::In<ac_int<64,true>> dma_write_chnl;

  SC_CTOR(Source) {
    SC_THREAD(run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    // some config variables
    int cfg_src_num_int = 16;
    int cfg_block_size_int = 8;

    float cfg_src_coeff_fp[16] = {
      0.5938,1.0841,0.7713,0.9023,1.3416,1.2921,0.6968,1.4938,
      1.2288,0.9707,1.1912,0.7459,0.9162,1.3737,0.6934,0.7728
    };

    float cfg_chan_coeff_fp[16] = {
      1.3528,1.0193,1.1141,0.5576,0.6451,0.5893,1.2548,1.0511,
      1.0671,0.8487,0.6927,1.3675,0.9941,0.7267,0.6743,1.4581    
    };

    // test only 128 audio inputs (-32767, 32767), i.e. 8 samples from each source
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





    // Reference output should be 8 samples from each channel, 16 channels
    // audio_in_int[sample][chan]
    float audio_out_fp[8][16];

    for (int t = 0; t < 8; t++) {       // sample
      for (int i = 0; i < 16; i++) {    // chan
        audio_out_fp[t][i] = 0;
        for (int j = 0; j < cfg_src_num_int; j++) {  // src
          audio_out_fp[t][i] += (audio_in_fp[t][j]/32768) * cfg_src_coeff_fp[j] * cfg_chan_coeff_fp[i];
        }
        cout << audio_out_fp[t][i] << "\t";
      }
    }

    // transpose and convert to int format
    int audio_in_int[16*8];
    for (int t = 0; t < 8; t++) {
      for (int i = 0; i < 16; i++) {    // src
        audio_in_int[i*8+t] = audio_in_fp[t][i];
      }
    }



    float audio_out_int[8*16];
    for (int t = 0; t < 8; t++) {       // sample
      for (int i = 0; i < 16; i++) {    // chan
        audio_out_int[t*16+i] =  (int) (audio_out_fp[t][i]*65536);
      }
    }



    // reset
    for (int i = 0; i < 48; i++) {
      cfg_regs[i].write(0);
    }

    acc_start.write(0);
    dma_read_ctrl.Reset();
    dma_read_chnl.Reset();
    dma_write_ctrl.Reset();
    dma_write_chnl.Reset();

    // Wait for initial reset
    wait(100, SC_NS);

    // set configs
    // Configurations 
    // 00:      N/A
    // 01:      cfg_src_num
    // 02:      cfg_block_size
    // 03:      cfg_input_base
    // 04:      cfg_output_base
    // 05-0F:   N/A
    // 10-1F:   cfg_src_coeff[0~F]
    // 20-2F:   cfg_chan_coeff[0~F]



    cfg_regs[1].write(cfg_src_num_int);
    cfg_regs[2].write(cfg_block_size_int);
    cfg_regs[3].write(1234);
    cfg_regs[4].write(4567);


    for (int i = 0; i < 16; i++) {
      ac_fixed<32,16,true> tmp0 = cfg_src_coeff_fp[i];
      cfg_regs[i+16].write(tmp0.slc<32>(0) );
      
      //cout << tmp0.slc<32>(0) << endl;
      ac_fixed<32,16,true> tmp1 = cfg_chan_coeff_fp[i];
      cfg_regs[i+32].write(tmp1.slc<32>(0) );
      //cout << tmp1.slc<32>(0) << endl;
    }

    wait(20, SC_NS);
    cout << "@" << sc_time_stamp() << " Start Computation " << endl ;

    // start signal 
    acc_start.write(1);
    cout << "@" << sc_time_stamp() << " acc_start.write(1) " << endl;
    wait();
    wait();
    acc_start.write(0);
    cout << "@" << sc_time_stamp() << " acc_start.write(0) " << endl;

    // DMA read 
    dma_info_t dma_info_read = dma_read_ctrl.Pop();
    cout << "@" << sc_time_stamp() << " Pop dma_info_read " << endl;

    // some verification on dma_info_read
    assert(dma_info_read.index == 1234); // same as cfg_input_base
    assert(dma_info_read.length == (cfg_src_num_int*cfg_block_size_int >> 2));
    assert(dma_info_read.size == 3);

    // simulate SoC latency
    wait(10, SC_NS);
    // Push DMA data 
    for (int i = 0; i < dma_info_read.length; i++) {
      ac_int<64,true> data;
      for (int j = 0; j < 4; j++) {

        ac_int<16,true> data_slc = audio_in_int[4*i + j];
//        cout << data_slc << ", ";
        data.set_slc(j*16, data_slc);  // Little Endian
      }
      dma_read_chnl.Push(data);
      cout << "@" << sc_time_stamp() << " Push DMA data " << endl;
    }

    // The accelerator should start computation after DMA read
    // DMA write
    dma_info_t dma_info_write = dma_write_ctrl.Pop();
    cout << "@" << sc_time_stamp() << " Pop dma_info_write " << endl;
    assert(dma_info_write.index == 4567); // same as cfg_input_base
    assert(dma_info_write.length == (16*cfg_block_size_int >> 1));
    assert(dma_info_write.size == 3);

    // simulate SoC latency
    wait(10, SC_NS);
    for (int i = 0; i < dma_info_write.length; i++) {
      ac_int<64,true> data = dma_write_chnl.Pop();
      cout << data.slc<32>(0) << ", " << data.slc<32>(32) << ", ";
      /* 
      int data0 = data.slc<32>(0);
      int ref0 = audio_out_int[2*i];
      cout << "@" << sc_time_stamp() << " Pop data0 = " << data0 
              << "\t" << "ref0 = " << ref0 << endl;


      int data1 = data.slc<32>(32);
      int ref1 = audio_out_int[2*i+1];
      cout << "@" << sc_time_stamp() << " Pop data1 = " << data1
              << "\t" << "ref1 = " << ref1 << endl;
    
      */
    } 

    // read signal
    while (acc_done.read() == 0) {
      wait();
    }
    cout << "@" << sc_time_stamp() << " acc_done.read() == 1 " << endl;
    wait(5); 



  }
};

SC_MODULE (testbench) {
  sc_clock clk;
  sc_signal<bool> rst;

  sc_signal<ac_int<32,true>>   cfg_regs[48];
  sc_signal<bool> acc_start;    // start signal
  sc_signal<bool> acc_done;    // done signal 

  // DMA ports 
  Connections::Combinational<dma_info_t> dma_read_ctrl;
  Connections::Combinational<ac_int<64,true>> dma_read_chnl;
  Connections::Combinational<dma_info_t> dma_write_ctrl;
  Connections::Combinational<ac_int<64,true>> dma_write_chnl;


  NVHLS_DESIGN(Top) dut;
  Source src;

  SC_HAS_PROCESS(testbench);
  testbench(sc_module_name name)
  : sc_module(name),
    clk("clk", 1, SC_NS, 0.5,0,SC_NS,true),
//    clk("clk", 2, SC_NS, 0.5,0,SC_NS,true),
    rst("rst"),
    dut("dut"),
    src("src")
  {
    
    dut.clk(clk);
    dut.rst(rst);
    for (int i = 0; i < 48; i++) {
      dut.cfg_regs[i](cfg_regs[i]);
    }
    dut.acc_start(acc_start);
    dut.acc_done(acc_done);
    dut.dma_read_ctrl(dma_read_ctrl);
    dut.dma_read_chnl(dma_read_chnl);
    dut.dma_write_ctrl(dma_write_ctrl);
    dut.dma_write_chnl(dma_write_chnl);


    src.clk(clk);
    src.rst(rst);
    for (int i = 0; i < 48; i++) {
      src.cfg_regs[i](cfg_regs[i]);
    }
    src.acc_start(acc_start);
    src.acc_done(acc_done);
    src.dma_read_ctrl(dma_read_ctrl);
    src.dma_read_chnl(dma_read_chnl);
    src.dma_write_ctrl(dma_write_ctrl);
    src.dma_write_chnl(dma_write_chnl);

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
