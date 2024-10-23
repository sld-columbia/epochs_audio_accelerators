/* Testing design */

#include "OutputBuffer.h"
#include <systemc.h>
#include <mc_scverify.h>

#define NVHLS_VERIFY_BLOCKS (OutputBuffer)
#include <nvhls_verify.h>
#include <testbench/nvhls_rand.h>
using namespace::std;


SC_MODULE (Source) {
  sc_in <bool> clk;
  sc_in <bool> rst;

  sc_out<ac_int<32,true>>   cfg_block_size;
  sc_out<ac_int<32,true>>   cfg_output_base; 

//  Connections::SyncOut  sync1;
  Connections::Out<bool>  sync2;
  Connections::In<bool>   sync3;
  Connections::In<dma_info_t> dma_write_ctrl;
  
  Connections::In<ac_int<64,true>> dma_write_chnl;
  Connections::Out<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>> audio_out;

  SC_CTOR(Source) {
    SC_THREAD(run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    // generate synthetic values
    int cfg_block_size_int = 8;

    // 8 samples, and 16 (hardcode) channels
    // => 16 channels*8 samples
    int audio_out_int[16*8] = {
      -10817,10253,-19607,22372,-11418,4086,222,-20823,
      -18070,16046,-2936,31292,-24151,24695,-5418,-940,
      10966,-29296,-18308,788,-29502,24359,-8808,22710,
      11538,18944,-6344,-20097,-212,-27484,21841,19304,
      -12084,25301,-10021,-25635,30548,-31592,9993,-928,
      -5927,14095,24909,360,21569,9148,-24430,13329,
      10039,-16789,7911,19719,-21548,9563,-2524,8258,
      -8955,-13160,2056,19472,1601,29961,3994,-2049,
      30813,-14230,14061,13067,20813,-30547,-14468,-27837,
      15913,-28020,-31262,32588,-13833,-11617,-7159,-1949,
      -12348,-17301,-13249,-17834,-3633,-26922,-23731,-24694,
      -12060,11720,-18056,5054,-15816,14476,-29303,-3207,
      17160,8348,3225,8915,3030,8009,12290,6957,
      30240,31077,21943,3723,-11993,23838,26628,-2445,
      28411,4588,5457,14659,-25891,10527,11932,12366,
      -5238,-31400,23241,28830,6928,20758,-304,-9499
    }; 
    
    // Reset
    cfg_block_size.write(0);
    cfg_output_base.write(0);

//    sync1.reset_sync_out();  // start signal
    sync2.Reset();
    sync3.Reset();
    dma_write_ctrl.Reset();
    dma_write_chnl.Reset();
    audio_out.Reset();

    // Wait for initial reset
    wait(100, SC_NS);

    // set configs
    cfg_block_size.write(cfg_block_size_int);
    cfg_output_base.write(1234);
    wait(20, SC_NS);
    
    cout << "@" << sc_time_stamp() << " Start Computation " << endl ;
    // start computation 
    //bool tmp = 1;
    sync2.Push(1);


    wait(10, SC_NS);
    // Push DMA data
    for (int i = 0; i < cfg_block_size_int; i++) {  // sample
      nvhls::nv_scvector<ac_fixed<32,16,true>, 16> audio_out_reg;
      for (int j = 0; j < 16; j++) {                // channel
        //ac_int<32,true> tmp_int = audio_out_int[16*i + j];
        ac_int<32,true> tmp_int = audio_out_int[i + cfg_block_size_int*j];  //transpose

        ac_fixed<32,16,true> tmp_fixed;
        tmp_fixed.set_slc(0, tmp_int);
        audio_out_reg[j] = tmp_fixed;
      }

      audio_out.Push(audio_out_reg);
      cout << "@" << sc_time_stamp() << " Push audio_out data " << endl;
    }


    dma_info_t dma_info_write = dma_write_ctrl.Pop();
    
    // some verification on dma_info_read
    assert(dma_info_write.index == 1234); // same as cfg_input_base
    assert(dma_info_write.length == (16*cfg_block_size_int >> 1));
    assert(dma_info_write.size == 3);

    wait(10, SC_NS);
    for (int i = 0; i < dma_info_write.length; i++) {
      ac_int<64,true> data = dma_write_chnl.Pop();

      int data0 = data.slc<32>(0);
      int ref0 = audio_out_int[2*i];
      cout << "@" << sc_time_stamp() << " Pop data0 = " << data0 
              << "\t" << "ref0 = " << ref0 << endl;


      int data1 = data.slc<32>(32);
      int ref1 = audio_out_int[2*i+1];
      cout << "@" << sc_time_stamp() << " Pop data1 = " << data1
              << "\t" << "ref1 = " << ref1 << endl;
    } 


    sync3.Pop(); // done
    cout << "@" << sc_time_stamp() << " Recieve Done sync3" << endl ;


    wait(5); 
  }// void run()
};

SC_MODULE (testbench) {
  sc_clock clk;
  sc_signal<bool> rst;

  sc_signal<ac_int<32,true>>   cfg_block_size;
  sc_signal<ac_int<32,true>>   cfg_output_base; 

//  Connections::SyncOut  sync1;
  Connections::Combinational<bool>  sync2;
  Connections::Combinational<bool>   sync3;
  Connections::Combinational<dma_info_t> dma_write_ctrl;
  
  Connections::Combinational<ac_int<64,true>> dma_write_chnl;
  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>> audio_out;

  NVHLS_DESIGN(OutputBuffer) dut;
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
    dut.cfg_block_size(cfg_block_size);
    dut.cfg_output_base(cfg_output_base);

    dut.sync2(sync2);
    dut.sync3(sync3);
    dut.dma_write_ctrl(dma_write_ctrl);
    dut.dma_write_chnl(dma_write_chnl);
    dut.audio_out(audio_out);



    src.clk(clk);
    src.rst(rst);
    src.cfg_block_size(cfg_block_size);
    src.cfg_output_base(cfg_output_base);

    src.sync2(sync2);
    src.sync3(sync3);
    src.dma_write_ctrl(dma_write_ctrl);
    src.dma_write_chnl(dma_write_chnl);
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