#ifndef _WDL_DENORMAL_H_
#define _WDL_DENORMAL_H_

typedef struct 
{ 
  #ifdef __ppc__ // todo: other big endian platforms...
    unsigned int hw; 
    unsigned int lw;
  #else
    unsigned int lw; 
    unsigned int hw;
  #endif
} WDL_DenormalTwoInts;

typedef union { double fl; WDL_DenormalTwoInts w; } WDL_DenormalDoubleAccess;
typedef union { float fl; unsigned int w; } WDL_DenormalFloatAccess;


// note: the _aggressive versions filter out anything less than around 1.0e-16 or so (approximately) to 0.0, including -0.0 (becomes 0.0)
// note: new! the _aggressive versions also filter inf and NaN to 0.0

#ifdef __cplusplus
#define WDL_DENORMAL_INLINE inline
#elif defined(_MSC_VER)
#define WDL_DENORMAL_INLINE __inline
#else
#define WDL_DENORMAL_INLINE
#endif

#define WDL_DENORMAL_DOUBLE_HW(a) (((const WDL_DenormalDoubleAccess*)(a))->w.hw)
#define WDL_DENORMAL_DOUBLE_LW(a) (((const WDL_DenormalDoubleAccess*)(a))->w.lw)
#define WDL_DENORMAL_FLOAT_W(a) (((const WDL_DenormalFloatAccess*)(a))->w)

#define WDL_DENORMAL_DOUBLE_HW_NC(a) (((WDL_DenormalDoubleAccess*)(a))->w.hw)
#define WDL_DENORMAL_DOUBLE_LW_NC(a) (((WDL_DenormalDoubleAccess*)(a))->w.lw)
#define WDL_DENORMAL_FLOAT_W_NC(a) (((WDL_DenormalFloatAccess*)(a))->w)

#define WDL_DENORMAL_DOUBLE_AGGRESSIVE_CUTOFF 0x3cA00000 // 0x3B8000000 maybe instead? that's 10^-5 smaller or so
#define WDL_DENORMAL_FLOAT_AGGRESSIVE_CUTOFF 0x25000000


// define WDL_DENORMAL_WANTS_SCOPED_FTZ, and then use a WDL_denormal_ftz_scope in addition to denormal_*(), then 
// if FTZ is available it will be used instead...
//
#ifdef WDL_DENORMAL_WANTS_SCOPED_FTZ

#if defined(__x86_64__) || _M_IX86_FP >= 2 || defined(__SSE2__) || defined(_WIN64)
  #define WDL_DENORMAL_SSEMODE
  #ifdef _MSC_VER
    #include <intrin.h>
  #else
    #include <xmmintrin.h>
  #endif
  #define wdl_denorm_mm_getcsr() _mm_getcsr() 
  #define wdl_denorm_mm_setcsr(x) _mm_setcsr(x) 
#endif

class WDL_denormal_ftz_scope 
{
  public:
    WDL_denormal_ftz_scope()
    {
#ifdef WDL_DENORMAL_SSEMODE
      const unsigned int b = (1<<15)|(1<<11);
      old_state = wdl_denorm_mm_getcsr();
      if ((need_restore = (old_state & b) != b))
          wdl_denorm_mm_setcsr(old_state|b);
#endif
    }
    ~WDL_denormal_ftz_scope()
    {
#ifdef WDL_DENORMAL_SSEMODE
      if (need_restore) wdl_denorm_mm_setcsr(old_state);
#endif
    }

#ifdef WDL_DENORMAL_SSEMODE
    unsigned int old_state;
    bool need_restore;
#endif

};


#endif


#if !defined(WDL_DENORMAL_SSEMODE) && !defined(WDL_DENORMAL_DO_NOT_FILTER)

static double WDL_DENORMAL_INLINE denormal_filter_double(double a)
{
  return (WDL_DENORMAL_DOUBLE_HW(&a)&0x7ff00000) ? a : 0.0;
}

static double WDL_DENORMAL_INLINE denormal_filter_double2(double a)
{
  return ((WDL_DENORMAL_DOUBLE_HW(&a)+0x100000)&0x7ff00000) > 0x100000 ? a : 0.0;
}

static double WDL_DENORMAL_INLINE denormal_filter_double_aggressive(double a)
{
  return ((WDL_DENORMAL_DOUBLE_HW(&a)+0x100000)&0x7ff00000) >= WDL_DENORMAL_DOUBLE_AGGRESSIVE_CUTOFF ? a : 0.0;
}

static float WDL_DENORMAL_INLINE denormal_filter_float(float a)
{
  return (WDL_DENORMAL_FLOAT_W(&a)&0x7f800000) ? a : 0.0f;
}

static float WDL_DENORMAL_INLINE denormal_filter_float2(float a)
{
  return ((WDL_DENORMAL_FLOAT_W(&a)+0x800000)&0x7f800000) > 0x800000 ? a : 0.0f; 
}


static float WDL_DENORMAL_INLINE denormal_filter_float_aggressive(float a)
{
  return ((WDL_DENORMAL_FLOAT_W(&a)+0x800000)&0x7f800000) >= WDL_DENORMAL_FLOAT_AGGRESSIVE_CUTOFF ? a : 0.0f; 
}
static void WDL_DENORMAL_INLINE denormal_fix_double(double *a)
{
  if (!(WDL_DENORMAL_DOUBLE_HW(a)&0x7ff00000)) *a=0.0;
}

static void WDL_DENORMAL_INLINE denormal_fix_double_aggressive(double *a)
{
  if (((WDL_DENORMAL_DOUBLE_HW(a)+0x100000)&0x7ff00000) < WDL_DENORMAL_DOUBLE_AGGRESSIVE_CUTOFF) *a=0.0;
}

static void WDL_DENORMAL_INLINE denormal_fix_float(float *a)
{
  if (!(WDL_DENORMAL_FLOAT_W(a)&0x7f800000)) *a=0.0f;
}
static void WDL_DENORMAL_INLINE denormal_fix_float_aggressive(float *a)
{
  if (((WDL_DENORMAL_FLOAT_W(a)+0x800000)&0x7f800000) < WDL_DENORMAL_FLOAT_AGGRESSIVE_CUTOFF) *a=0.0f;
}



#ifdef __cplusplus // automatic typed versions (though one should probably use the explicit versions...


static double WDL_DENORMAL_INLINE denormal_filter(double a)
{
  return (WDL_DENORMAL_DOUBLE_HW(&a)&0x7ff00000) ? a : 0.0;
}
static double WDL_DENORMAL_INLINE denormal_filter_aggressive(double a)
{
  return ((WDL_DENORMAL_DOUBLE_HW(&a)+0x100000)&0x7ff00000) >= WDL_DENORMAL_DOUBLE_AGGRESSIVE_CUTOFF ? a : 0.0;
}

static float WDL_DENORMAL_INLINE denormal_filter(float a)
{
  return (WDL_DENORMAL_FLOAT_W(&a)&0x7f800000) ? a : 0.0f;
}

static float WDL_DENORMAL_INLINE denormal_filter_aggressive(float a)
{
  return ((WDL_DENORMAL_FLOAT_W(&a)+0x800000)&0x7f800000) >= WDL_DENORMAL_FLOAT_AGGRESSIVE_CUTOFF ? a : 0.0f;
}

static void WDL_DENORMAL_INLINE denormal_fix(double *a)
{
  if (!(WDL_DENORMAL_DOUBLE_HW(a)&0x7ff00000)) *a=0.0;
}
static void WDL_DENORMAL_INLINE denormal_fix_aggressive(double *a)
{
  if (((WDL_DENORMAL_DOUBLE_HW(a)+0x100000)&0x7ff00000) < WDL_DENORMAL_DOUBLE_AGGRESSIVE_CUTOFF) *a=0.0;
}
static void WDL_DENORMAL_INLINE denormal_fix(float *a)
{
  if (!(WDL_DENORMAL_FLOAT_W(a)&0x7f800000)) *a=0.0f;
}
static void WDL_DENORMAL_INLINE denormal_fix_aggressive(float *a)
{
  if (((WDL_DENORMAL_FLOAT_W(a)+0x800000)&0x7f800000) < WDL_DENORMAL_FLOAT_AGGRESSIVE_CUTOFF) *a=0.0f;
}



#endif // cplusplus versions

#else // end of !WDL_DENORMAL_DO_NOT_FILTER (and other platform-specific checks)

#define denormal_filter_double(x) (x)
#define denormal_filter_double2(x) (x)
#define denormal_filter_double_aggressive(x) (x)
#define denormal_filter_float(x) (x)
#define denormal_filter_float2(x) (x)
#define denormal_filter_float_aggressive(x) (x)
#define denormal_filter_aggressive(x) (x)
#define denormal_fix(x) do { } while(0)
#define denormal_fix_aggressive(x) do { } while(0)
#define denormal_fix_double(x) do { } while(0)
#define denormal_fix_double_aggressive(x) do { } while(0)
#define denormal_fix_float(x) do { } while(0)
#define denormal_fix_float_aggressive(x) do { } while(0)

#endif 


////////////////////
// this isnt a denormal function but it is similar, so we'll put it here as a bonus

static void WDL_DENORMAL_INLINE GetDoubleMaxAbsValue(double *out, const double *in) // note: the value pointed to by "out" must be >=0.0, __NOT__ <= -0.0
{
  unsigned int hw = WDL_DENORMAL_DOUBLE_HW(in)&0x7fffffff;
  if (hw >= WDL_DENORMAL_DOUBLE_HW(out) && (hw>WDL_DENORMAL_DOUBLE_HW(out) || WDL_DENORMAL_DOUBLE_LW(in) > WDL_DENORMAL_DOUBLE_LW(out)))
  {
    WDL_DENORMAL_DOUBLE_LW_NC(out) = WDL_DENORMAL_DOUBLE_LW(in);
    WDL_DENORMAL_DOUBLE_HW_NC(out) = hw;
  }
}

static void WDL_DENORMAL_INLINE GetFloatMaxAbsValue(float *out, const float *in) // note: the value pointed to by "out" must be >=0.0, __NOT__ <= -0.0
{
  unsigned int hw = WDL_DENORMAL_FLOAT_W(in)&0x7fffffff;
  if (hw > WDL_DENORMAL_FLOAT_W(out)) WDL_DENORMAL_FLOAT_W_NC(out)=hw;
}


#ifdef __cplusplus
static void WDL_DENORMAL_INLINE GetFloatMaxAbsValue(double *out, const double *in) // note: the value pointed to by "out" must be >=0.0, __NOT__ <= -0.0
{
  GetDoubleMaxAbsValue(out,in);
}
#endif

#endif
