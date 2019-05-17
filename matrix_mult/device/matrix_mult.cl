/*OpenCL C*/
//AOCX:/home/pbarua3/tmp/t1394283161.aocx 
#pragma OPENCL FP_CONTRACT ON
#pragma OPENCL EXTENSION cl_intel_channels : enable
float float_from_bits(unsigned int x) {return as_float(x);}
float nan_f32() { return NAN; }
float neg_inf_f32() { return -INFINITY; }
float inf_f32() { return INFINITY; }
#define sqrt_f32 sqrt 
#define sin_f32 sin 
#define cos_f32 cos 
#define exp_f32 exp 
#define log_f32 log 
#define abs_f32 fabs 
#define floor_f32 floor 
#define ceil_f32 ceil 
#define round_f32 round 
#define trunc_f32 trunc 
#define pow_f32 pow
#define asin_f32 asin 
#define acos_f32 acos 
#define tan_f32 tan 
#define atan_f32 atan 
#define atan2_f32 atan2
#define sinh_f32 sinh 
#define asinh_f32 asinh 
#define cosh_f32 cosh 
#define acosh_f32 acosh 
#define tanh_f32 tanh 
#define atanh_f32 atanh 
#define fast_inverse_f32 native_recip 
#define fast_inverse_sqrt_f32 native_rsqrt 
int halide_gpu_thread_barrier() {
  barrier(CLK_LOCAL_MEM_FENCE);
  return 0;
}
#define __address_space___shared __local
//_A_min_0 =0,  _A_min_1=0,  _A_min_2=0,        _A_stride_1=12,  _A_stride_2=144,         _inputA_extent_2=2,   _inputA_min_0=2,         _inputA_min_1 = 2,  _inputA_min_2=1,         _inputA_stride_1=8,  _inputA_stride_2=64 
//0.0+20.0=20.0 0.0+20.0=40.0 20.0+20.0=80.0 30.0+0.0=110.0 40.0+0.0=150.0 72=150.0
//t=1 row=7,col=3  input[193]=0.0 0.0+30.0=30.0 20.0+30.0=80.0 30.0+30.0=140.0 40.0+0.0=180.0 50.0+0.0=230.0 73=230.0
//t=1 row=7,col=4  input[194]=0.0 20.0+40.0=60.0 30.0+40.0=130.0 40.0+40.0=210.0 50.0+0.0=260.0 60.0+0.0=320.0 74=320.0
//t=1 row=7,col=5  input[195]=0.0 30.0+50.0=80.0 40.0+50.0=170.0 50.0+50.0=270.0 60.0+0.0=330.0 70.0+0.0=400.0 75=400.0
//t=1 row=7,col=6  input[196]=0.0 40.0+60.0=100.0 50.0+60.0=210.0 60.0+60.0=330.0 70.0+0.0=400.0 0.0+0.0=400.0 76=400.0
//t=1 row=7,col=7  input[197]=0.0 50.0+70.0=120.0 60.0+70.0=250.0 70.0+70.0=390.0 0.0+0.0=390.0 0.0+0.0=390.0 77=390.0
//
//
//
// Address spaces for kernel_inputA__run_on_device
#define DEBUG 0
#define NUMCOLS 128
#define RADIUS 2
#define STENCILSIZE (2*RADIUS*NUMCOLS+1)
#define __address_space__A __global
#define __address_space__inputA __global
__attribute__((reqd_work_group_size(1, 1, 1)))
__kernel void kernel_inputA_run_on_device(
 const int _A_min_0,
 const int _A_min_1,
 const int _A_min_2,
 const int _A_stride_1,
 const int _A_stride_2,
 const int _inputA_extent_0,
 const int _inputA_extent_1,
 const int _inputA_extent_2,
 const int _inputA_min_0,
 const int _inputA_min_1,
 const int _inputA_min_2,
 const int _inputA_stride_1,
 const int _inputA_stride_2,
 __address_space__A const float *_A,
 __address_space__inputA float *_inputA
)
{
  /*if (DEBUG) printf("_A_min_0 =%d,  _A_min_1=%d,  _A_min_2=%d, \
       _A_stride_1=%d,  _A_stride_2=%d, \
        _inputA_extent_2=%d,   _inputA_min_0=%d, \
        _inputA_min_1 = %d,  _inputA_min_2=%d,\
         _inputA_stride_1=%d,  _inputA_stride_2=%d",_A_min_0,_A_min_1,_A_min_2,_A_stride_1,_A_stride_2,_inputA_extent_2,_inputA_min_0,_inputA_min_1,
         _inputA_min_2,_inputA_stride_1,_inputA_stride_2);*/

 float _shiftReg_A_stencilRadius_0_stencil[STENCILSIZE];
 (void)0;
 uint _0 = (uint)(_inputA_extent_1 );
 for (int _inputA_s0_row = 0; _inputA_s0_row < 0 + _0; _inputA_s0_row++)
 {
  uint _1 = (uint)(_inputA_extent_0);
  for (int _inputA_s0_col = 0; _inputA_s0_col < 0 + _1; _inputA_s0_col++)
  {
   uint _2 = (uint)(_inputA_stride_1);
   int _3 = _inputA_s0_row * _2;
   int _4 = _inputA_s0_col + _3;
   int _5 = 0 * _2;
   int _6 = 0 + _5;
   int _7 = 0 * _A_stride_2;
   int _8 = _6 + _7;
   int _9 = _4 - _8;
   float _10 = _A[_9];
   uint _11 = (uint)(_inputA_stride_2);
   int _12 = 0 * _11;
   int _13 = _6 + _12;
   int _14 = _4 - _13;
   _inputA[_14] = _10;
   if (DEBUG) printf(" %d=%.1f ", _14, _10);
  } // for _inputA_s0_col
  if (DEBUG) printf("\n");
 } // for _inputA_s0_row
 for (int _inputA_s1_time = 0; _inputA_s1_time < 0 + _inputA_extent_2; _inputA_s1_time++)
 {
  for (int _inputA_s1_row = -RADIUS; _inputA_s1_row < -RADIUS + _inputA_extent_1 ; _inputA_s1_row++)
  {
   for (int _inputA_s1_col = 0; _inputA_s1_col < 0 + _inputA_extent_0; _inputA_s1_col++)
   {
     if (DEBUG) printf(" t=%d row=%d,col=%d ",_inputA_s1_time , _inputA_s1_row, _inputA_s1_col );
    
 #pragma unroll 
     for (int shiftI = 1 ; shiftI < STENCILSIZE; shiftI++) {
       _shiftReg_A_stencilRadius_0_stencil[shiftI-1]=_shiftReg_A_stencilRadius_0_stencil[shiftI];
     }
        int _15 = _inputA_s1_row + RADIUS;
    uint _16 = (uint)(_inputA_stride_1);
    int _17 = _15 * _16;
    int _18 = _inputA_s1_col + _17;
    int _19 = _inputA_s1_time & 1;
    uint _20 = (uint)(_inputA_stride_2);
    int _21 = _19 * _20;
    int _22 = _18 + _21;
    int _23 = 0 * _16;
    int _24 = 0 + _23;
    int _25 = 0 * _20;
    int _26 = _24 + _25;
    int _27 = _22 - _26;
    float _28 = _inputA[_27];

_shiftReg_A_stencilRadius_0_stencil[STENCILSIZE-1]=_28;
    (void)0;
    bool _29 = RADIUS <= _inputA_s1_col;
    bool _30 = _inputA_s1_col <= (_inputA_extent_0 - RADIUS -1) ;
    bool _31 = _29 && _30;
    bool _32 = RADIUS <= _inputA_s1_row;
    bool _33 = _inputA_s1_row <= (_inputA_extent_1 - RADIUS -1);
    bool _34 = _32 && _33;
    bool _35 = _31 && _34;
    if (DEBUG) printf(" input[%d]=%.1f, %d ", _27, _28, _35);
    if (_35)
    {
      if (DEBUG) printf("Y");
     float _reduction_temp_updateinputA;
          _reduction_temp_updateinputA = 0;
 #pragma unroll 
     for (int _inputA_s1_stencilRadius__x = -RADIUS; _inputA_s1_stencilRadius__x < -RADIUS + (2*RADIUS+1); _inputA_s1_stencilRadius__x++)
     {
       int _36 = _inputA_s1_stencilRadius__x + RADIUS*_inputA_extent_0;
       float _37 = _shiftReg_A_stencilRadius_0_stencil[_36];
       int _38 = _inputA_s1_stencilRadius__x * _inputA_extent_0;
       int _39 = _38 + RADIUS*_inputA_extent_0;
       float _40 = _shiftReg_A_stencilRadius_0_stencil[_39];
       float _41 = _37 + _40;
       float _42 = _reduction_temp_updateinputA + _41;
       _reduction_temp_updateinputA = _42;
       //if (DEBUG) printf(" %.1f+%.1f=%.1f",_37, _40, _42);
     } // for _inputA_s1_stencilRadius__x
     uint _43 = (uint)(_inputA_stride_1);
     int _44 = _inputA_s1_row * _43;
     int _45 = _inputA_s1_col + _44;
     int _46 = _inputA_s1_time + 1;
     int _47 = _46 & 1;
     uint _48 = (uint)(_inputA_stride_2);
     int _49 = _47 * _48;
     int _50 = _45 + _49;
     int _51 = 0 * _43;
     int _52 = 0 + _51;
     int _53 = 0 * _48;
     int _54 = _52 + _53;
     int _55 = _50 - _54;
     _inputA[_55] = _reduction_temp_updateinputA;
     if (DEBUG) printf(" %d=%.1f", _55, _reduction_temp_updateinputA);
    } // if _35
   } // for _inputA_s1_col
   if (DEBUG) printf("\n");
  } // for _inputA_s1_row
 } // for _inputA_s1_time

   if (DEBUG) printf("\n  DONE \n");
} // kernel kernel_inputA__run_on_device
#undef __address_space__A
#undef __address_space__inputA
