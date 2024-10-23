#ifndef __Common_H__
#define __Common_H__

#include <systemc.h>
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_module.h>
//#include <nvhls_connections.h>
#include <mc_connections.h>
#include <nvhls_message.h>

struct dma_info_t: public nvhls_message{
  public:
  // Index
  ac_int<32, false> index;
  // Length
  ac_int<32, false> length; 
  // Word Size =  SIZE_DWORD  = 3, 8*2^3 = 64
  ac_int<3, false> size;    

  static const unsigned int width = 32+32+3;
  
  template <unsigned int Size>
  void Marshall(Marshaller<Size>& m) {
    m& index;   //lsb
    m& length;
    m& size;    //msb
  }
};



#endif
