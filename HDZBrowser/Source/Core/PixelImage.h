#pragma once
#include <unordered_map>
#include <string>
#include "bgfx\bgfx.h"

enum PixelCategory
{
    UnknownBinary = 0,
    WAVFile,
    CharacterName,
    BMPFile,

    COUNT
};

class PixelImage
{
public:
    struct ColorEntry
    {
        uint32_t color;   // RGBA color value
        std::string name; // Name of the color
    };

    PixelImage( size_t width, size_t height, const std::unordered_map<uint8_t, ColorEntry>& colorTable );

    void SetPixel( size_t index, PixelCategory colorKey );
    void SetPixelRange( size_t start, size_t end, PixelCategory colorKey );
    void Resize( size_t newWidth, size_t newHeight );

    void WriteImage( const std::string& filepath );

    void UploadToGPU();
    void RenderImGui();

    size_t GetLength();

private:
    size_t width, height;
    std::vector<uint8_t> pixelIndices; // Indexes into colorTable
    std::unordered_map<uint8_t, ColorEntry> colorTable;
    std::unordered_map<uint8_t, uint32_t> colorCount;

    bgfx::TextureHandle texture;
    bool gpuDirty = true; // Flag to update texture if modified
};
