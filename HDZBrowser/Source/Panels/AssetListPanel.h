#pragma once
#include "Core\HeadDef.h"
#include "Core\PixelImage.h"

class AssetListPanel
{
public:
    AssetListPanel();

    void Draw();

    void Init();
private:
    std::vector<HeadDef> m_headList;
    std::vector<HeadDef> m_deadHeadList;
    PixelImage pixelImage;
};