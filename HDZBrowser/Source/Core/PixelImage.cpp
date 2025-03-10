#include "PixelImage.h"
#include "Utils\ImGuiUtils.h"

PixelImage::PixelImage( size_t w, size_t h, const std::unordered_map<uint8_t, ColorEntry>& table )
    : width( w ), height( h ), colorTable( table ), texture(BGFX_INVALID_HANDLE)
{
    pixelIndices.resize( w * h, 0 ); // Default all pixels to the first color key
}


void PixelImage::SetPixel( size_t index, PixelCategory colorKey )
{
    if( index < pixelIndices.size() )
    {
        pixelIndices[index] = colorKey;
        gpuDirty = true;
        colorCount[colorKey] += 1;
    }
}

void PixelImage::SetPixelRange( size_t start, size_t end, PixelCategory colorKey )
{
    if( start >= end || start >= pixelIndices.size() || end > pixelIndices.size() )
        return;

    std::fill( pixelIndices.begin() + start, pixelIndices.begin() + end, colorKey );
    colorCount[colorKey]++;// = end - start;
    gpuDirty = true;
}

void PixelImage::Resize( size_t newWidth, size_t newHeight )
{
    std::vector<uint8_t> newPixelIndices( newWidth * newHeight );

    // Nearest-neighbor resampling
    for( size_t y = 0; y < newHeight; ++y )
    {
        for( size_t x = 0; x < newWidth; ++x )
        {
            size_t oldX = ( x * width ) / newWidth;
            size_t oldY = ( y * height ) / newHeight;
            newPixelIndices[y * newWidth + x] = pixelIndices[oldY * width + oldX];
        }
    }
    colorCount.clear();
    // Update image properties
    width = newWidth;
    height = newHeight;
    pixelIndices = std::move( newPixelIndices );
    gpuDirty = true;
}

void PixelImage::WriteImage( const std::string& filepath )
{
    //std::vector<uint32_t> pixelData( width * height );
    //for( size_t i = 0; i < pixelIndices.size(); ++i )
    //{
    //    pixelData[i] = colorTable[pixelIndices[i]];
    //}
    //
    //stbi_write_png( filepath.c_str(), width, height, 4, pixelData.data(), width * 4 );
}

void PixelImage::UploadToGPU()
{
    if( !gpuDirty ) return;

    std::vector<uint32_t> pixelData( width * height );
    for( size_t i = 0; i < pixelIndices.size(); ++i )
    {
        pixelData[i] = colorTable[pixelIndices[i]].color;
    }
    if( bgfx::isValid( texture ) )
    {
        bgfx::destroy( texture );
    }
    texture = bgfx::createTexture2D(
        static_cast<uint16_t>( width ), static_cast<uint16_t>( height ),
        false, 1, bgfx::TextureFormat::RGBA8, 0,
        bgfx::copy( pixelData.data(), pixelData.size() * sizeof( uint32_t ) )
    );

    gpuDirty = false;
}

void PixelImage::RenderImGui()
{
    UploadToGPU();

    // Display the image
    ImGui::Image( (void*)(uintptr_t)texture.idx, ImVec2( width, height ) );

    // Display the color key legend
    ImGui::Begin( "Color Key" );
    for( const auto& [key, entry] : colorTable )
    {
        ImGui::Text( "%s: Count: %i", entry.name.c_str(), colorCount[key]);

        ImGui::SameLine();
        ImVec4 color = ImVec4(
            ( ( entry.color >> 16 ) & 0xFF ) / 255.0f,
            ( ( entry.color >> 8 ) & 0xFF ) / 255.0f,
            ( ( entry.color >> 0 ) & 0xFF ) / 255.0f,
            ( ( entry.color >> 24 ) & 0xFF ) / 255.0f
        );
        ImGui::ColorButton( ( "##color" + std::to_string( key ) ).c_str(), color );
    }
    ImGui::End();
}


size_t PixelImage::GetLength()
{
    return pixelIndices.size();
}

