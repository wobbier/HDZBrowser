#pragma once
#include <string>
#include <vector>

struct HeadDef
{
    HeadDef() = default;
    std::string RawID;
    std::string ID;
    int CharacterIndex = 0;
    std::vector<std::string> AssociatedAudioFiles; // Temp for now
    std::vector<std::string> HeadPortraits;
};