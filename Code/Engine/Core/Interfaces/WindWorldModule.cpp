#include <CorePCH.h>

#include <Core/Interfaces/WindWorldModule.h>
#include <World/World.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezWindWorldModuleInterface, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezWindStrength, 1)
  EZ_ENUM_CONSTANTS(ezWindStrength::Calm, ezWindStrength::LightBreeze, ezWindStrength::GentleBreeze, ezWindStrength::ModerateBreeze, ezWindStrength::StrongBreeze, ezWindStrength::Storm)
EZ_END_STATIC_REFLECTED_ENUM;
// clang-format on

float ezWindStrength::GetInMetersPerSecond(Enum strength)
{
  // inspired by the Beaufort scale
  // https://en.wikipedia.org/wiki/Beaufort_scale

  switch (strength)
  {
    case Calm:
      return 0.5f;

    case LightBreeze:
      return 2.0f;

    case GentleBreeze:
      return 5.0f;

    case ModerateBreeze:
      return 9.0f;

    case StrongBreeze:
      return 14.0f;

    case Storm:
      return 20.0f;

      EZ_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return 0;
}

ezWindWorldModuleInterface::ezWindWorldModuleInterface(ezWorld* pWorld)
  : ezWorldModule(pWorld)
{
}

ezVec3 ezWindWorldModuleInterface::ComputeWindFlutter(const ezVec3& vWind, const ezVec3& vObjectDir, float fFlutterSpeed, ezUInt32 uiFlutterRandomOffset) const
{
  if (vWind.IsZero(0.001f))
    return ezVec3::ZeroVector();

  ezVec3 windDir = vWind;
  const float fWindStrength = windDir.GetLengthAndNormalize();

  if (fWindStrength <= 0.01f)
    return ezVec3::ZeroVector();

  ezVec3 mainDir = vObjectDir;
  mainDir.NormalizeIfNotZero(ezVec3::UnitZAxis()).IgnoreResult();

  ezVec3 flutterDir = windDir.CrossRH(mainDir);
  flutterDir.NormalizeIfNotZero(ezVec3::UnitZAxis()).IgnoreResult();

  const float fFlutterOffset = (uiFlutterRandomOffset & 1023u) / 256.0f;

  const float fFlutter = ezMath::Sin(ezAngle::Radian(fFlutterOffset + fFlutterSpeed * fWindStrength * GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds())) * fWindStrength;

  return flutterDir * fFlutter;
}

EZ_STATICLINK_FILE(Core, Core_Interfaces_WindWorldModule);