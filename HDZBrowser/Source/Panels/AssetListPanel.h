#pragma once
#include "Core\HeadDef.h"

class AssetListPanel
{
public:
    AssetListPanel() = default;

    void Draw();

private:
    std::vector<HeadDef> m_headList;
    std::vector<HeadDef> m_deadHeadList;
};