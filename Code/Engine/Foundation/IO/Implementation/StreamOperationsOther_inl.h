#pragma once

/// \brief Operator to serialize ezIAllocator::Stats objects.
EZ_FOUNDATION_DLL void operator<<(ezStreamWriter& out_stream, const ezAllocatorBase::Stats& rhs);

/// \brief Operator to serialize ezIAllocator::Stats objects.
EZ_FOUNDATION_DLL void operator>>(ezStreamReader& out_stream, ezAllocatorBase::Stats& rhs);

struct ezTime;

/// \brief Operator to serialize ezTime objects.
EZ_FOUNDATION_DLL void operator<<(ezStreamWriter& out_stream, ezTime value);

/// \brief Operator to serialize ezTime objects.
EZ_FOUNDATION_DLL void operator>>(ezStreamReader& out_stream, ezTime& out_value);


class ezUuid;

/// \brief Operator to serialize ezUuid objects. [tested]
EZ_FOUNDATION_DLL void operator<<(ezStreamWriter& out_stream, const ezUuid& value);

/// \brief Operator to serialize ezUuid objects. [tested]
EZ_FOUNDATION_DLL void operator>>(ezStreamReader& out_stream, ezUuid& out_value);

class ezHashedString;

/// \brief Operator to serialize ezHashedString objects. [tested]
EZ_FOUNDATION_DLL void operator<<(ezStreamWriter& out_stream, const ezHashedString& sValue);

/// \brief Operator to serialize ezHashedString objects. [tested]
EZ_FOUNDATION_DLL void operator>>(ezStreamReader& out_stream, ezHashedString& out_sValue);

class ezTempHashedString;

/// \brief Operator to serialize ezHashedString objects.
EZ_FOUNDATION_DLL void operator<<(ezStreamWriter& out_stream, const ezTempHashedString& sValue);

/// \brief Operator to serialize ezHashedString objects.
EZ_FOUNDATION_DLL void operator>>(ezStreamReader& out_stream, ezTempHashedString& out_sValue);

class ezVariant;

/// \brief Operator to serialize ezVariant objects.
EZ_FOUNDATION_DLL void operator<<(ezStreamWriter& out_stream, const ezVariant& value);

/// \brief Operator to serialize ezVariant objects.
EZ_FOUNDATION_DLL void operator>>(ezStreamReader& out_stream, ezVariant& out_value);

class ezTimestamp;

/// \brief Operator to serialize ezTimestamp objects.
EZ_FOUNDATION_DLL void operator<<(ezStreamWriter& out_stream, ezTimestamp value);

/// \brief Operator to serialize ezTimestamp objects.
EZ_FOUNDATION_DLL void operator>>(ezStreamReader& out_stream, ezTimestamp& out_value);

struct ezVarianceTypeFloat;

/// \brief Operator to serialize ezTimestamp objects.
EZ_FOUNDATION_DLL void operator<<(ezStreamWriter& out_stream, const ezVarianceTypeFloat& value);

/// \brief Operator to serialize ezTimestamp objects.
EZ_FOUNDATION_DLL void operator>>(ezStreamReader& out_stream, ezVarianceTypeFloat& out_value);

struct ezVarianceTypeTime;

/// \brief Operator to serialize ezTimestamp objects.
EZ_FOUNDATION_DLL void operator<<(ezStreamWriter& out_stream, const ezVarianceTypeTime& value);

/// \brief Operator to serialize ezTimestamp objects.
EZ_FOUNDATION_DLL void operator>>(ezStreamReader& out_stream, ezVarianceTypeTime& out_value);

struct ezVarianceTypeAngle;

/// \brief Operator to serialize ezTimestamp objects.
EZ_FOUNDATION_DLL void operator<<(ezStreamWriter& out_stream, const ezVarianceTypeAngle& value);

/// \brief Operator to serialize ezTimestamp objects.
EZ_FOUNDATION_DLL void operator>>(ezStreamReader& out_stream, ezVarianceTypeAngle& out_value);
