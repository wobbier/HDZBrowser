#pragma once
#include <string>
#include <vector>


struct HeadDef
{
    // 12 Bytes
    struct HeaderInfo
    {
        uint8_t HeaderID = 0x02;
        uint8_t UnknownU8; // The HeaderID could be a u16 and not a part of this header section to make room below for another u32
        uint32_t DataSize; // Seems to be the size of the data not including the header size (12 bytes)
        uint32_t OtherData;
        uint16_t UnknownU16ORU32; // There might be a case where the HeaderID is a u16
    };

    HeadDef() = default;
    HeaderInfo Header;
    std::string RawID;
    std::string ID;
    int CharacterIndex = 0;
    std::vector<std::string> AssociatedAudioFiles; // Temp for now
    std::vector<std::string> HeadPortraits;

    // Localization
    std::string EnglishLocKey;
    std::string SpanishLocKey;
    std::string ItalianLocKey;
    std::string FrenchLocKey;
    std::string DutchLocKey;
    std::string SwedishLocKey;
    std::string ExtraLocKey; // ??
};