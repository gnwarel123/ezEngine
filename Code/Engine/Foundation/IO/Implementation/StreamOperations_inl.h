#pragma once

#include <Foundation/Containers/Bitfield.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/Enum.h>

// Standard operators for overloads of common data types

/// bool versions

inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, bool bValue)
{
  ezUInt8 uiValue = bValue ? 1 : 0;
  out_stream.WriteBytes(&uiValue, sizeof(ezUInt8)).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, bool& out_bValue)
{
  ezUInt8 uiValue = 0;
  out_stream.ReadBytes(&uiValue, sizeof(ezUInt8));
  out_bValue = (uiValue != 0);
  return out_stream;
}

/// unsigned int versions

inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, ezUInt8 uiValue)
{
  out_stream.WriteBytes(&uiValue, sizeof(ezUInt8)).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, ezUInt8& out_uiValue)
{
  out_stream.ReadBytes(&out_uiValue, sizeof(ezUInt8));
  return out_stream;
}

inline ezResult SerializeArray(ezStreamWriter& out_stream, const ezUInt8* pArray, ezUInt64 uiCount)
{
  return out_stream.WriteBytes(pArray, sizeof(ezUInt8) * uiCount);
}

inline ezResult DeserializeArray(ezStreamReader& out_stream, ezUInt8* pArray, ezUInt64 uiCount)
{
  const ezUInt64 uiNumBytes = sizeof(ezUInt8) * uiCount;
  if (out_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return EZ_SUCCESS;

  return EZ_FAILURE;
}


inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, ezUInt16 uiValue)
{
  out_stream.WriteWordValue(&uiValue).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, ezUInt16& out_uiValue)
{
  out_stream.ReadWordValue(&out_uiValue).IgnoreResult();
  return out_stream;
}

inline ezResult SerializeArray(ezStreamWriter& out_stream, const ezUInt16* pArray, ezUInt64 uiCount)
{
  return out_stream.WriteBytes(pArray, sizeof(ezUInt16) * uiCount);
}

inline ezResult DeserializeArray(ezStreamReader& out_stream, ezUInt16* pArray, ezUInt64 uiCount)
{
  const ezUInt64 uiNumBytes = sizeof(ezUInt16) * uiCount;
  if (out_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return EZ_SUCCESS;

  return EZ_FAILURE;
}


inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, ezUInt32 uiValue)
{
  out_stream.WriteDWordValue(&uiValue).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, ezUInt32& out_uiValue)
{
  out_stream.ReadDWordValue(&out_uiValue).IgnoreResult();
  return out_stream;
}

inline ezResult SerializeArray(ezStreamWriter& out_stream, const ezUInt32* pArray, ezUInt64 uiCount)
{
  return out_stream.WriteBytes(pArray, sizeof(ezUInt32) * uiCount);
}

inline ezResult DeserializeArray(ezStreamReader& out_stream, ezUInt32* pArray, ezUInt64 uiCount)
{
  const ezUInt64 uiNumBytes = sizeof(ezUInt32) * uiCount;
  if (out_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return EZ_SUCCESS;

  return EZ_FAILURE;
}


inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, ezUInt64 uiValue)
{
  out_stream.WriteQWordValue(&uiValue).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, ezUInt64& out_uiValue)
{
  out_stream.ReadQWordValue(&out_uiValue).IgnoreResult();
  return out_stream;
}

inline ezResult SerializeArray(ezStreamWriter& out_stream, const ezUInt64* pArray, ezUInt64 uiCount)
{
  return out_stream.WriteBytes(pArray, sizeof(ezUInt64) * uiCount);
}

inline ezResult DeserializeArray(ezStreamReader& out_stream, ezUInt64* pArray, ezUInt64 uiCount)
{
  const ezUInt64 uiNumBytes = sizeof(ezUInt64) * uiCount;
  if (out_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return EZ_SUCCESS;

  return EZ_FAILURE;
}

/// signed int versions

inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, ezInt8 iValue)
{
  out_stream.WriteBytes(reinterpret_cast<const ezUInt8*>(&iValue), sizeof(ezInt8)).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, ezInt8& out_iValue)
{
  out_stream.ReadBytes(reinterpret_cast<ezUInt8*>(&out_iValue), sizeof(ezInt8));
  return out_stream;
}

inline ezResult SerializeArray(ezStreamWriter& out_stream, const ezInt8* pArray, ezUInt64 uiCount)
{
  return out_stream.WriteBytes(pArray, sizeof(ezInt8) * uiCount);
}

inline ezResult DeserializeArray(ezStreamReader& out_stream, ezInt8* pArray, ezUInt64 uiCount)
{
  const ezUInt64 uiNumBytes = sizeof(ezInt8) * uiCount;
  if (out_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return EZ_SUCCESS;

  return EZ_FAILURE;
}


inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, ezInt16 iValue)
{
  out_stream.WriteWordValue(&iValue).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, ezInt16& out_iValue)
{
  out_stream.ReadWordValue(&out_iValue).IgnoreResult();
  return out_stream;
}

inline ezResult SerializeArray(ezStreamWriter& out_stream, const ezInt16* pArray, ezUInt64 uiCount)
{
  return out_stream.WriteBytes(pArray, sizeof(ezInt16) * uiCount);
}

inline ezResult DeserializeArray(ezStreamReader& out_stream, ezInt16* pArray, ezUInt64 uiCount)
{
  const ezUInt64 uiNumBytes = sizeof(ezInt16) * uiCount;
  if (out_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return EZ_SUCCESS;

  return EZ_FAILURE;
}


inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, ezInt32 iValue)
{
  out_stream.WriteDWordValue(&iValue).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, ezInt32& out_iValue)
{
  out_stream.ReadDWordValue(&out_iValue).IgnoreResult();
  return out_stream;
}

inline ezResult SerializeArray(ezStreamWriter& out_stream, const ezInt32* pArray, ezUInt64 uiCount)
{
  return out_stream.WriteBytes(pArray, sizeof(ezInt32) * uiCount);
}

inline ezResult DeserializeArray(ezStreamReader& out_stream, ezInt32* pArray, ezUInt64 uiCount)
{
  const ezUInt64 uiNumBytes = sizeof(ezInt32) * uiCount;
  if (out_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return EZ_SUCCESS;

  return EZ_FAILURE;
}


inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, ezInt64 iValue)
{
  out_stream.WriteQWordValue(&iValue).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, ezInt64& out_iValue)
{
  out_stream.ReadQWordValue(&out_iValue).IgnoreResult();
  return out_stream;
}

inline ezResult SerializeArray(ezStreamWriter& out_stream, const ezInt64* pArray, ezUInt64 uiCount)
{
  return out_stream.WriteBytes(pArray, sizeof(ezInt64) * uiCount);
}

inline ezResult DeserializeArray(ezStreamReader& out_stream, ezInt64* pArray, ezUInt64 uiCount)
{
  const ezUInt64 uiNumBytes = sizeof(ezInt64) * uiCount;
  if (out_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return EZ_SUCCESS;

  return EZ_FAILURE;
}


/// float and double versions

inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, float fValue)
{
  out_stream.WriteDWordValue(&fValue).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, float& out_fValue)
{
  out_stream.ReadDWordValue(&out_fValue).IgnoreResult();
  return out_stream;
}

inline ezResult SerializeArray(ezStreamWriter& out_stream, const float* pArray, ezUInt64 uiCount)
{
  return out_stream.WriteBytes(pArray, sizeof(float) * uiCount);
}

inline ezResult DeserializeArray(ezStreamReader& out_stream, float* pArray, ezUInt64 uiCount)
{
  const ezUInt64 uiNumBytes = sizeof(float) * uiCount;
  if (out_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return EZ_SUCCESS;

  return EZ_FAILURE;
}


inline ezStreamWriter& operator<<(ezStreamWriter& out_stream, double fValue)
{
  out_stream.WriteQWordValue(&fValue).IgnoreResult();
  return out_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& out_stream, double& out_fValue)
{
  out_stream.ReadQWordValue(&out_fValue).IgnoreResult();
  return out_stream;
}

inline ezResult SerializeArray(ezStreamWriter& out_stream, const double* pArray, ezUInt64 uiCount)
{
  return out_stream.WriteBytes(pArray, sizeof(double) * uiCount);
}

inline ezResult DeserializeArray(ezStreamReader& out_stream, double* pArray, ezUInt64 uiCount)
{
  const ezUInt64 uiNumBytes = sizeof(double) * uiCount;
  if (out_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return EZ_SUCCESS;

  return EZ_FAILURE;
}


// C-style strings
// No read equivalent for C-style strings (but can be read as ezString & ezStringBuilder instances)

EZ_FOUNDATION_DLL ezStreamWriter& operator<<(ezStreamWriter& out_stream, const char* szValue);

// ezHybridString

template <ezUInt16 Size, typename AllocatorWrapper>
inline ezStreamWriter& operator<<(ezStreamWriter& stream, const ezHybridString<Size, AllocatorWrapper>& sValue)
{
  stream.WriteString(sValue.GetView()).IgnoreResult();
  return stream;
}

template <ezUInt16 Size, typename AllocatorWrapper>
inline ezStreamReader& operator>>(ezStreamReader& stream, ezHybridString<Size, AllocatorWrapper>& sValue)
{
  ezStringBuilder builder;
  stream.ReadString(builder).IgnoreResult();
  sValue = std::move(builder);

  return stream;
}

// ezStringBuilder

EZ_FOUNDATION_DLL ezStreamWriter& operator<<(ezStreamWriter& out_stream, const ezStringBuilder& sValue);
EZ_FOUNDATION_DLL ezStreamReader& operator>>(ezStreamReader& out_stream, ezStringBuilder& out_sValue);

// ezEnum

template <typename T>
inline ezStreamWriter& operator<<(ezStreamWriter& stream, const ezEnum<T>& value)
{
  stream << value.GetValue();

  return stream;
}

template <typename T>
inline ezStreamReader& operator>>(ezStreamReader& stream, ezEnum<T>& value)
{
  typename T::StorageType storedValue = T::Default;
  stream >> storedValue;
  value.SetValue(storedValue);

  return stream;
}

// ezBitflags

template <typename T>
inline ezStreamWriter& operator<<(ezStreamWriter& stream, const ezBitflags<T>& value)
{
  stream << value.GetValue();

  return stream;
}

template <typename T>
inline ezStreamReader& operator>>(ezStreamReader& stream, ezBitflags<T>& value)
{
  typename T::StorageType storedValue = T::Default;
  stream >> storedValue;
  value.SetValue(storedValue);

  return stream;
}
