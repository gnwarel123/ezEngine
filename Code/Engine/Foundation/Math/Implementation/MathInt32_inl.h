#pragma once

namespace ezMath
{
  constexpr EZ_ALWAYS_INLINE ezInt32 RoundUp(ezInt32 iValue, ezUInt16 uiMultiple)
  {
    //
    return (iValue >= 0) ? ((iValue + uiMultiple - 1) / uiMultiple) * uiMultiple : (iValue / uiMultiple) * uiMultiple;
  }

  constexpr EZ_ALWAYS_INLINE ezInt32 RoundDown(ezInt32 iValue, ezUInt16 uiMultiple)
  {
    //
    return (iValue <= 0) ? ((iValue - uiMultiple + 1) / uiMultiple) * uiMultiple : (iValue / uiMultiple) * uiMultiple;
  }

  constexpr EZ_ALWAYS_INLINE ezUInt32 RoundUp(ezUInt32 uiValue, ezUInt16 uiMultiple)
  {
    //
    return ((uiValue + uiMultiple - 1) / uiMultiple) * uiMultiple;
  }

  constexpr EZ_ALWAYS_INLINE ezUInt32 RoundDown(ezUInt32 uiValue, ezUInt16 uiMultiple)
  {
    //
    return (uiValue / uiMultiple) * uiMultiple;
  }

  constexpr EZ_ALWAYS_INLINE bool IsOdd(ezInt32 i)
  {
    //
    return ((i & 1) != 0);
  }

  constexpr EZ_ALWAYS_INLINE bool IsEven(ezInt32 i)
  {
    //
    return ((i & 1) == 0);
  }

  inline ezUInt32 Log2i(ezUInt32 uiVal)
  {
    ezInt32 ret = -1;
    while (uiVal != 0)
    {
      uiVal >>= 1;
      ret++;
    }

    return (ezUInt32)ret;
  }

  constexpr EZ_ALWAYS_INLINE int Pow2(int i)
  {
    //
    return (1 << i);
  }

  inline int Pow(int iBase, int iExp)
  {
    int res = 1;
    while (iExp > 0)
    {
      res *= iBase;
      --iExp;
    }

    return res;
  }

} // namespace ezMath
