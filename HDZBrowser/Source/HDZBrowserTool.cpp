#include "HDZBrowserTool.h"

#include <imgui.h>
#include <Path.h>
#include <Graphics/Texture.h>
#include <Resource/ResourceCache.h>
#include "Utils/ImGuiUtils.h"
#include <utility>

#include <optional>
#include <Mathf.h>
#include "Utils/PlatformUtils.h"
#include "Dementia.h"
//#include "Utils/HUBUtils.h"
#include "Config.h"
#include "Window/SDLWindow.h"


HDZBrowserTool::HDZBrowserTool( ToolCreationFlags& inToolCreationFlags )
    : Tool( inToolCreationFlags )
{
}


void HDZBrowserTool::OnStart()
{
    AssetPanel.Init();
}

void HDZBrowserTool::OnUpdate()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    TitleBarDragSize = { viewport->Size.x, 50.f };
//#if USING( ME_PLATFORM_WIN64 )
//    if( m_input.WasKeyPressed( KeyCode::A ) )
//    {
//        HUB::ShowOpenFilePrompt( m_window->GetWindowPtr() );
//    }
//#endif
    AssetPanel.Draw();

    //static bool showDemo = true;
    //ImGui::ShowDemoWindow( &showDemo );
}