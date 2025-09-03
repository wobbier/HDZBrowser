#include "AssetListPanel.h"
#include "imgui.h"
#include "Utils\HDZUtils.h"

void DrawHeadItem( HeadDef& inHead )
{
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
    std::string headerWithIndex = std::to_string( inHead.CharacterIndex ) + ": " + inHead.ID;
    if( ImGui::TreeNodeEx( headerWithIndex.c_str(), node_flags ) )
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

AssetListPanel::AssetListPanel()
    : pixelImage( 128, 128, {
    {PixelCategory::UnknownBinary, {0xFF000000, "Unknown"}},
    {PixelCategory::WAVFile, {0xFFFF0000, "WAV File"}},
    {PixelCategory::CharacterName, {0xFF00FF00, "Character Name"}},
    {PixelCategory::BMPFile, {0xFF0000FF, "BMP File"}},
    {PixelCategory::SolvedData, {0xFFFFFFFF, "Solved Data"}},
        } )
{

}


void AssetListPanel::Init()
{
    // Example color definitions:
    pixelImage.Resize( 3000, 9000 ); // Scale up 2x
    //pixelImage.SetPixel( 10, 1 ); // Set pixel index 10 to Red
    //pixelImage.SetPixelRange( 20, 30, 2 ); // Set pixels [20, 30) to Green

    pixelImage.UploadToGPU();
    pixelImage.WriteImage( "output.png" );
}


void AssetListPanel::Draw()
{

    ImGui::Begin( "BIN PREVIEW" );
    pixelImage.RenderImGui(); // Call inside ImGui window
    ImGui::End();
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

        HDZUtils::parse_hdz_file( "Assets/hedz.hdz", m_headList, m_deadHeadList, pixelImage );
    }

    std::vector<Path> binFiles = {
        // AMB Music
        Path( "Assets/HEDZ/MUSIC/1.AMB" ),
        Path( "Assets/HEDZ/MUSIC/2.AMB" ),
        Path( "Assets/HEDZ/MUSIC/3.AMB" ),
        Path( "Assets/HEDZ/MUSIC/4.AMB" ),
        Path( "Assets/HEDZ/MUSIC/5.AMB" ),
        Path( "Assets/HEDZ/MUSIC/6.AMB" ),
        Path( "Assets/HEDZ/MUSIC/7.AMB" ),
        // Levels
        Path( "Assets/RAW/MapFiles/C_Dtown.cmp" ),
        Path( "Assets/RAW/MapFiles/C_Locks.cmp" ),
        Path( "Assets/RAW/MapFiles/E_Chase.cmp" ),
        Path( "Assets/RAW/MapFiles/E_Dogfig.cmp" ),
        Path( "Assets/RAW/MapFiles/E_Rise.cmp" ),
        Path( "Assets/RAW/MapFiles/E_Siege.cmp" ),
        Path( "Assets/RAW/MapFiles/I_Machin.cmp" ),
        Path( "Assets/RAW/MapFiles/I_Prodct.cmp" ),
        Path( "Assets/RAW/MapFiles/I_WareH.cmp" ),
        Path( "Assets/RAW/MapFiles/K_Bricks.cmp" ),
        Path( "Assets/RAW/MapFiles/K_FnFair.cmp" ),
        Path( "Assets/RAW/MapFiles/K_Park.cmp" ),
        Path( "Assets/RAW/MapFiles/M_Alien.cmp" ),
        Path( "Assets/RAW/MapFiles/M_City.cmp" ),
        Path( "Assets/RAW/MapFiles/M_Dogfig.cmp" ),
        Path( "Assets/RAW/MapFiles/M_Indust.cmp" ),
        Path( "Assets/RAW/MapFiles/M_Kid.cmp" ),
        Path( "Assets/RAW/MapFiles/M_Occult.cmp" ),
        Path( "Assets/RAW/MapFiles/M_Scienc.cmp" ),
        Path( "Assets/RAW/MapFiles/M_War.cmp" ),
        Path( "Assets/RAW/MapFiles/O_Fort.cmp" ),
        Path( "Assets/RAW/MapFiles/O_House.cmp" ),
        Path( "Assets/RAW/MapFiles/O_Infrno.cmp" ),
        Path( "Assets/RAW/MapFiles/S_Mole.cmp" ),
        Path( "Assets/RAW/MapFiles/S_Weird.cmp" ),
        Path( "Assets/RAW/MapFiles/W_Enemy.cmp" ),
        Path( "Assets/RAW/MapFiles/W_Jungle.cmp" ),
        Path( "Assets/RAW/MapFiles/W_NoMan.cmp" ),
        //Misc
        Path( "Assets/HEDZ/fxdata.dat" ),
        Path( "Assets/HEDZ/fedata.dat" ),
        Path( "Assets/HEDZ/HEDZ.EXE" ),
        Path( "Assets/HEDZ/SoundFX.dat" ),
    };

    if( ImGui::Button( "Check for Audio Headers" ) )
    {
        for (auto& path : binFiles)
        {
            HDZUtils::scanAndExtractAudioFiles( path );
        }
    }

    if( ImGui::Button( "Parse Map File" ) )
    {
        HDZUtils::parse_map_file( binFiles );
    }

    if( ImGui::Button( "Parse Bin File" ) )
    {
        HDZUtils::Smain();
    }

    std::string totalHeads( "Total Heads: " + std::to_string( m_headList.size() ) + "/225" );
    ImGui::Text( totalHeads.c_str() );

    if( ImGui::CollapsingHeader( "Fixed Heads", 0 ) )
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