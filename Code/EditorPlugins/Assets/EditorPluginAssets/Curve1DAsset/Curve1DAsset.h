#pragma once

#include <EditorFramework/Assets/SimpleAssetDocument.h>
#include <GuiFoundation/Widgets/CurveEditData.h>

class ezCurve1D;

class ezCurve1DAssetDocument : public ezSimpleAssetDocument<ezCurveGroupData>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezCurve1DAssetDocument, ezSimpleAssetDocument<ezCurveGroupData>);

public:
  ezCurve1DAssetDocument(const char* szDocumentPath);
  ~ezCurve1DAssetDocument();

  /// \brief Fills out the ezCurve1D structure with an exact copy of the data in the asset.
  /// Does NOT yet sort the control points, so before evaluating the curve, that must be called manually.
  void FillCurve(ezUInt32 uiCurveIdx, ezCurve1D& out_Result) const;

  ezUInt32 GetCurveCount() const;

  void WriteResource(ezStreamWriter& stream) const;

protected:
  virtual ezTransformStatus InternalTransformAsset(ezStreamWriter& stream, const char* szOutputTag, const ezPlatformProfile* pAssetProfile,
    const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags) override;
  virtual ezTransformStatus InternalCreateThumbnail(const ThumbnailInfo& ThumbnailInfo) override;
};
