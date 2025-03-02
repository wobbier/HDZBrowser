#pragma once
#include <Pointers.h>
#include "Tool.h"
#include "Panels\AssetListPanel.h"

class HDZBrowserTool
    : public Tool
{
public:
    HDZBrowserTool( ToolCreationFlags& inToolCreationFlags );

    void OnUpdate() final;

    void OnStart() override;

private:
    AssetListPanel AssetPanel;
};
