#include "AssetListPanel.h"
#include "imgui.h"
#include "Utils\HDZUtils.h"

void DrawHeadItem( HeadDef& inHead )
{
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
    if( ImGui::TreeNodeEx( inHead.ID.c_str(), node_flags ) )
    {
        for( auto& child : inHead.AssociatedAudioFiles )
        {
            node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if( ImGui::TreeNodeEx( child.c_str(), node_flags ) )
            {
                // do more
            }
        }
        for( auto& child : inHead.HeadPortraits )
        {
            node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if( ImGui::TreeNodeEx( child.c_str(), node_flags ) )
            {
                // do more
            }
        }
        ImGui::TreePop();
    }
}

void AssetListPanel::Draw()
{
    ImGui::Begin( "Character List" );

    if( ImGui::Button( "Parse HDZ File" ) )
    {
        std::vector<std::string> testStrings = {
            "Bare_AlienBARE_ALIENBARE_ALIENBARE_ALIEN__",
            "Angry_demonstratorANGRY_DEMONSTRATORMANIFESTANTE_ENFADADO...",
            "Aircraft_carrier_headAIRCRAFT-CARRIER_HEAD...",
            "33399",
            "XXXXXXXX",
            "DDD",
            "888",
            "Qnn"
        };

        for( auto& str : testStrings )
        {
            std::string extracted = HDZUtils::GetCharacterID( str );
            std::cout << "FINAL RESULT: " << extracted << std::endl;
        }

        HDZUtils::parse_hdz_file( "Assets/hedz.hdz", m_headList, m_deadHeadList );
    }

    std::string totalHeads( "Total Heads: " + std::to_string(m_headList.size()) + "/279");
    ImGui::Text( totalHeads.c_str() );

    if( ImGui::CollapsingHeader( "Fixed Heads", 0) )
    {
        for( auto& head : m_headList )
        {
            DrawHeadItem( head );
        }
    }

    if( ImGui::CollapsingHeader( "Unfixed Heads", 0 ) )
    {
        for( auto& head : m_deadHeadList )
        {
            DrawHeadItem( head );
        }
    }

    ImGui::End();
}
