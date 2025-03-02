#include "AssetListPanel.h"
#include "imgui.h"
#include "Utils\HDZUtils.h"

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

        HDZUtils::extract_wav_files( "Assets/hedz.hdz" );
    }

    ImGui::End();
}
