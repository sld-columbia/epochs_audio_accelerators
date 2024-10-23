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

  sc_out<ac_int<32,true>>   cfg_regs[32];  

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
    //int cfg_src_num_int = 16;
    int cfg_block_size_int = 8;


    for (int i = 0; i < 32; i++) {
      cfg_regs[i].write(0);
    }

    acc_start.write(0);
    dma_read_ctrl.Reset();
    dma_read_chnl.Reset();
    dma_write_ctrl.Reset();
    dma_write_chnl.Reset();


    // compute reference input/output
    const float cos_alpha_fp =  0.6014;
    const float sin_alpha_fp =  0.9303;
    const float cos_beta_fp  = -0.7021;
    const float sin_beta_fp  = -0.8568;
    const float cos_gamma_fp = -0.5455;
    const float sin_gamma_fp = -0.3376;

    const float cos_2_alpha_fp = -0.6014;
    const float sin_2_alpha_fp =  0.8703;
    const float cos_2_beta_fp  = -0.2321;
    const float sin_2_beta_fp  =  0.1568;
    const float cos_2_gamma_fp =  0.4225;
    const float sin_2_gamma_fp = -0.7432;

    const float cos_3_alpha_fp = -0.6514;
    const float sin_3_alpha_fp = -0.1723;
    const float cos_3_beta_fp  =  0.7327;
    const float sin_3_beta_fp  =  0.3528;
    const float cos_3_gamma_fp = -0.8116;
    const float sin_3_gamma_fp =  0.3491;


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
      //float x1, y1, z1;       
      //float x2, y2, z2; 
      float Y0, Z0, X0;
      float Y1, Z1, X1;
      float Y2, Z2, X2;
      float Y3, Z3, X3;

      Y0 = rotate1_in_fp[i][0];
      Z0 = rotate1_in_fp[i][1];
      X0 = rotate1_in_fp[i][2];

      Y1 = -X0*sin_alpha_fp + Y0*cos_alpha_fp;
      Z1 = Z0;
      X1 =  X0*cos_alpha_fp + Y0*sin_alpha_fp;

      Y2 =  Y1;
      Z2 =  X1*sin_beta_fp + Z1*cos_beta_fp;
      X2 =  X1*cos_beta_fp + Z1*sin_beta_fp;    

      Y3 = -X2*sin_gamma_fp + Y2*cos_gamma_fp;
      Z3 =  Z2;
      X3 =  X2*cos_gamma_fp + Y2*sin_gamma_fp;

      rotate1_out_fp[i][0] = Y3;
      rotate1_out_fp[i][1] = Z3;
      rotate1_out_fp[i][2] = X3;

      //x1 =  rotate1_in_fp[i][0]*cos_alpha_fp + rotate1_in_fp[i][1]*sin_alpha_fp;
      //y1 = -rotate1_in_fp[i][0]*sin_alpha_fp + rotate1_in_fp[i][1]*cos_alpha_fp;
      //z1 = rotate1_in_fp[i][2];

      //x2 = x1*cos_beta_fp + z1*sin_beta_fp;
      //y2 = y1;
      //z2 = x1*sin_beta_fp + z1*cos_beta_fp;

      //rotate1_out_fp[i][0] =  x2*cos_gamma_fp + y2*sin_gamma_fp;
      //rotate1_out_fp[i][1] = -x2*sin_gamma_fp + y2*cos_gamma_fp;
      //rotate1_out_fp[i][2] =  z2;
    }

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

      V0 = rotate2_in_fp[i][0];
      T0 = rotate2_in_fp[i][1];
      R0 = rotate2_in_fp[i][2];
      S0 = rotate2_in_fp[i][3];
      U0 = rotate2_in_fp[i][4];

      //R0 = rotate2_in_fp[i][0];
      //S0 = rotate2_in_fp[i][1];
      //T0 = rotate2_in_fp[i][2];
      //U0 = rotate2_in_fp[i][3];
      //V0 = rotate2_in_fp[i][4];



      V1 = -U0*sin_2_alpha_fp + V0*cos_2_alpha_fp;
      T1 = -S0*sin_alpha_fp   + T0*cos_alpha_fp;
      R1 =  R0;
      S1 =  S0*cos_alpha_fp   + T0*sin_alpha_fp;
      U1 =  U0*cos_2_alpha_fp + V0*sin_2_alpha_fp;

      cout << V1 << "\t" << T1 << "\t" << R1 << "\t" << S1 << "\t" << U1 << endl; 


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
       
      rotate2_out_fp[i][0] = V3;
      rotate2_out_fp[i][1] = T3;
      rotate2_out_fp[i][2] = R3;
      rotate2_out_fp[i][3] = S3;
      rotate2_out_fp[i][4] = U3;
    }
  
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

    // rotate in/out fixed point format
    ac_fixed<32,16,true>  rotate_in_int[8*16];
    ac_fixed<32,16,true>  rotate_out_int[8*16];
    for (int i = 0; i < 8; i++) {
      rotate_in_int[i*16+ 0]  = 0;
      rotate_in_int[i*16+ 1]  = rotate1_in_fp[i][0];  
      rotate_in_int[i*16+ 2]  = rotate1_in_fp[i][1];
      rotate_in_int[i*16+ 3]  = rotate1_in_fp[i][2];      
      rotate_in_int[i*16+ 4]  = rotate2_in_fp[i][0];
      rotate_in_int[i*16+ 5]  = rotate2_in_fp[i][1];
      rotate_in_int[i*16+ 6]  = rotate2_in_fp[i][2];
      rotate_in_int[i*16+ 7]  = rotate2_in_fp[i][3];   
      rotate_in_int[i*16+ 8]  = rotate2_in_fp[i][4];
      rotate_in_int[i*16+ 9]  = rotate3_in_fp[i][0];
      rotate_in_int[i*16+10]  = rotate3_in_fp[i][1];
      rotate_in_int[i*16+11]  = rotate3_in_fp[i][2];   
      rotate_in_int[i*16+12]  = rotate3_in_fp[i][3];
      rotate_in_int[i*16+13]  = rotate3_in_fp[i][4];
      rotate_in_int[i*16+14]  = rotate3_in_fp[i][5];
      rotate_in_int[i*16+15]  = rotate3_in_fp[i][6];   

      rotate_out_int[i*16+ 0] = 0;
      rotate_out_int[i*16+ 1] = rotate1_out_fp[i][0]; 
      rotate_out_int[i*16+ 2] = rotate1_out_fp[i][1];
      rotate_out_int[i*16+ 3] = rotate1_out_fp[i][2];      
      rotate_out_int[i*16+ 4] = rotate2_out_fp[i][0];
      rotate_out_int[i*16+ 5] = rotate2_out_fp[i][1];
      rotate_out_int[i*16+ 6] = rotate2_out_fp[i][2];
      rotate_out_int[i*16+ 7] = rotate2_out_fp[i][3];  
      rotate_out_int[i*16+ 8] = rotate2_out_fp[i][4];
      rotate_out_int[i*16+ 9] = rotate3_out_fp[i][0];
      rotate_out_int[i*16+10] = rotate3_out_fp[i][1];
      rotate_out_int[i*16+11] = rotate3_out_fp[i][2];
      rotate_out_int[i*16+12] = rotate3_out_fp[i][3];
      rotate_out_int[i*16+13] = rotate3_out_fp[i][4];
      rotate_out_int[i*16+14] = rotate3_out_fp[i][5];
      rotate_out_int[i*16+15] = rotate3_out_fp[i][6];

    }



    // Wait for initial reset
    wait(100, SC_NS);
  // Configurations 
  // 00:      N/A
  // 01:      N/A
  // 02:      cfg_block_size
  // 03:      cfg_input_base
  // 04:      cfg_output_base
  // 05-07:   N/A
  // 08:      cfg_cos_alpha;
  // 09:      cfg_sin_alpha;  
  // 0A:      cfg_cos_beta;
  // 0B:      cfg_sin_beta;
  // 0C:      cfg_cos_gamma;
  // 0D:      cfg_sin_gamma;  
  // 0E:      cfg_cos_2_alpha;
  // 0F:      cfg_sin_2_alpha;
  // 10:      cfg_cos_2_beta;
  // 11:      cfg_sin_2_beta;
  // 12:      cfg_cos_2_gamma;
  // 13:      cfg_sin_2_gamma;
  // 14:      cfg_cos_3_alpha;
  // 15:      cfg_sin_3_alpha;
  // 16:      cfg_cos_3_beta;
  // 17:      cfg_sin_3_beta;
  // 18:      cfg_cos_3_gamma;
  // 19:      cfg_sin_3_gamma;
    
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 16; j++) {
        cout << rotate_in_int[i*16 + j].slc<32>(0) << ", ";
      }
       cout << endl;
    }



    //cfg_regs[1].write(cfg_src_num_int);
    cfg_regs[2].write(cfg_block_size_int);
    cfg_regs[3].write(1234);
    cfg_regs[4].write(4567);


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

    cfg_regs[ 8].write(cos_alpha_fixed.slc<32>(0));
    cfg_regs[ 9].write(sin_alpha_fixed.slc<32>(0));
    cfg_regs[10].write( cos_beta_fixed.slc<32>(0));
    cfg_regs[11].write( sin_beta_fixed.slc<32>(0));
    cfg_regs[12].write(cos_gamma_fixed.slc<32>(0));
    cfg_regs[13].write(sin_gamma_fixed.slc<32>(0));

    cfg_regs[14].write(cos_2_alpha_fixed.slc<32>(0));
    cfg_regs[15].write(sin_2_alpha_fixed.slc<32>(0));
    cfg_regs[16].write(cos_2_beta_fixed.slc<32>(0));
    cfg_regs[17].write(sin_2_beta_fixed.slc<32>(0));
    cfg_regs[18].write(cos_2_gamma_fixed.slc<32>(0));
    cfg_regs[19].write(sin_2_gamma_fixed.slc<32>(0));

    cfg_regs[20].write(cos_3_alpha_fixed.slc<32>(0));
    cfg_regs[21].write(sin_3_alpha_fixed.slc<32>(0));
    cfg_regs[22].write(cos_3_beta_fixed.slc<32>(0));
    cfg_regs[23].write(sin_3_beta_fixed.slc<32>(0));
    cfg_regs[24].write(cos_3_gamma_fixed.slc<32>(0));
    cfg_regs[25].write(sin_3_gamma_fixed.slc<32>(0));

 //   wait(10, SC_NS);
 //   for (int i = 8; i <=25; i++) cout << cfg_regs[i] << endl;


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
    assert(dma_info_read.length == (16*cfg_block_size_int >> 1));
    assert(dma_info_read.size == 3);

    // simulate SoC latency
    wait(10, SC_NS);
    // Push DMA data 
    for (int i = 0; i < dma_info_read.length; i++) {
      ac_int<64,true> data;
      for (int j = 0; j < 2; j++) {
        ac_int<32,true> data_slc = rotate_in_int[i*2 + j].slc<32>(0);
        //cout << data_slc << endl;
        data.set_slc(j*32, data_slc);  // Little Endian
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

      int data0 = data.slc<32>(0);
      int ref0 = rotate_out_int[2*i].slc<32>(0);
      cout << "@" << sc_time_stamp() << " Pop data0 (INT) = " << data0 
              << "\t" << "ref0 (INT) = " << ref0 << endl;


      int data1 = data.slc<32>(32);
      int ref1 = rotate_out_int[2*i+1].slc<32>(0);
      cout << "@" << sc_time_stamp() << " Pop data1 (INT) = " << data1
              << "\t" << "ref1 (INT) = " << ref1 << endl;
      
      //cout << data0 << ", "  << data1 << ", ";
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

  sc_signal<ac_int<32,true>>   cfg_regs[32];
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
    for (int i = 0; i < 32; i++) {
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
    for (int i = 0; i < 32; i++) {
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
