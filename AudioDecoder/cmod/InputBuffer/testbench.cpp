/* Testing design */

#include "InputBuffer.h"
#include <systemc.h>
#include <mc_scverify.h>

#define NVHLS_VERIFY_BLOCKS (InputBuffer)
#include <nvhls_verify.h>
#include <testbench/nvhls_rand.h>
using namespace::std;


SC_MODULE (Source) {
  sc_in <bool> clk;
  sc_in <bool> rst;

  //sc_out<ac_int<32,true>>   cfg_src_num;
  sc_out<ac_int<32,true>>   cfg_block_size;
  sc_out<ac_int<32,true>>   cfg_input_base; 

  Connections::Out<bool>  sync1;
  Connections::In<dma_info_t> dma_read_ctrl;
  
  Connections::Out<ac_int<64,true>> dma_read_chnl;
  Connections::In<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>>  rotate_in;

  SC_CTOR(Source) {
    SC_THREAD(run);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void run() {
    
    int cfg_block_size_int = 8;

    // 8 samples and 16 cahnnels
    int rotate_in_int[8*16] = {
      -10817,10253,-19607,22372,-11418,4086,222,-20823,-18070,16046,-2936,31292,-24151,24695,-5418,-940,
      10966,-29296,-18308,788,-29502,24359,-8808,22710,11538,18944,-6344,-20097,-212,-27484,21841,19304,
      -12084,25301,-10021,-25635,30548,-31592,9993,-928,-5927,14095,24909,360,21569,9148,-24430,13329,
      10039,-16789,7911,19719,-21548,9563,-2524,8258,-8955,-13160,2056,19472,1601,29961,3994,-2049,
      30813,-14230,14061,13067,20813,-30547,-14468,-27837,15913,-28020,-31262,32588,-13833,-11617,-7159,-1949,
      -12348,-17301,-13249,-17834,-3633,-26922,-23731,-24694,-12060,11720,-18056,5054,-15816,14476,-29303,-3207,
      17160,8348,3225,8915,3030,8009,12290,6957,30240,31077,21943,3723,-11993,23838,26628,-2445,
      28411,4588,5457,14659,-25891,10527,11932,12366,-5238,-31400,23241,28830,6928,20758,-304,-9499
    }; 
    
    // Reset
    cfg_block_size.write(0);
    cfg_input_base.write(0);

    sync1.Reset();  // start signal
    dma_read_ctrl.Reset();
    dma_read_chnl.Reset();
    rotate_in.Reset();

    // Wait for initial reset
    wait(100, SC_NS);

    // set configs
    cfg_block_size.write(cfg_block_size_int);
    cfg_input_base.write(1234);
    wait(20, SC_NS);

    cout << "@" << sc_time_stamp() << " Start Computation " << endl ;

    sync1.Push(1);
    wait();
    dma_info_t dma_info_read = dma_read_ctrl.Pop();

    assert(dma_info_read.index == 1234); // same as cfg_input_base
    assert(dma_info_read.length == (cfg_block_size_int >> 1)*16 );
    assert(dma_info_read.size == 3);

    wait(10, SC_NS);
    for (int i = 0; i < dma_info_read.length; i++) {
      ac_int<64,true> data;
      for (int j = 0; j < 2; j++) {
        ac_int<32,true> data_slc = rotate_in_int[2*i + j];
        //cout << data_slc << endl;
        data.set_slc(j*32, data_slc);  // Little Endian
      }
      dma_read_chnl.Push(data);
      cout << "@" << sc_time_stamp() << " Push DMA data " << endl;
    }

    wait(10, SC_NS);
    for (int i = 0; i < cfg_block_size_int; i++) {
      nvhls::nv_scvector<ac_fixed<32,16,true>, 16> out_reg;
      out_reg = rotate_in.Pop();
      for (int j = 0; j < 16; j++) {
        int ref = rotate_in_int[16*i + j]; // FIXME need to bitcast to fixed point
        cout << "@" << sc_time_stamp() << " Pop out_reg[" << j << "] = " << out_reg[j] 
             << "\t" << "ref = " << ref << endl;
      }
    }

    wait(5); 
  }
};

SC_MODULE (testbench) {
  sc_clock clk;
  sc_signal<bool> rst;

  sc_signal<ac_int<32,true>>   cfg_block_size;
  sc_signal<ac_int<32,true>>   cfg_input_base; 

//  Connections::SyncChannel  sync1; 
  // do not use SyncChannel, as the handshaking mechanism is different 
  Connections::Combinational<bool>  sync1;
  Connections::Combinational<dma_info_t> dma_read_ctrl;
  
  Connections::Combinational<ac_int<64,true>> dma_read_chnl;
  Connections::Combinational<nvhls::nv_scvector<ac_fixed<32,16,true>, 16>>  rotate_in;

  NVHLS_DESIGN(InputBuffer) dut;
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
    dut.cfg_input_base(cfg_input_base);

    dut.sync1(sync1);
    dut.dma_read_ctrl(dma_read_ctrl);
    dut.dma_read_chnl(dma_read_chnl);
    dut.rotate_in(rotate_in);



    src.clk(clk);
    src.rst(rst);
    src.cfg_block_size(cfg_block_size);
    src.cfg_input_base(cfg_input_base);

    src.sync1(sync1);
    src.dma_read_ctrl(dma_read_ctrl);
    src.dma_read_chnl(dma_read_chnl);
    src.rotate_in(rotate_in);
    
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