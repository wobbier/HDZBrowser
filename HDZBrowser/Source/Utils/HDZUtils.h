#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include "Core\Assert.h"
#include <algorithm>
#include "Path.h"
#include "CLog.h"
#include <vector>
#include <cstring>


#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <iomanip>

namespace HDZUtils
{
#pragma pack(push, 1)
    struct BITMAPFILEHEADER
    {
        uint16_t bfType;        // "BM" = 0x4D42
        uint32_t bfSize;        // File size in bytes
        uint16_t bfReserved1;   // Reserved (must be 0)
        uint16_t bfReserved2;   // Reserved (must be 0)
        uint32_t bfOffBits;     // Offset to pixel data
    };

    struct BITMAPINFOHEADER
    {
        uint32_t biSize;        // Size of this header (40 bytes)
        int32_t biWidth;        // Width of image
        int32_t biHeight;       // Height of image
        uint16_t biPlanes;      // Number of color planes (must be 1)
        uint16_t biBitCount;    // Bits per pixel (e.g., 24 for RGB)
        uint32_t biCompression; // Compression type (0 = uncompressed)
        uint32_t biSizeImage;   // Image data size (can be 0 for uncompressed)
        int32_t biXPelsPerMeter; // Pixels per meter X
        int32_t biYPelsPerMeter; // Pixels per meter Y
        uint32_t biClrUsed;      // Number of colors used
        uint32_t biClrImportant; // Important colors count
    };
#pragma pack(pop)

// Known texture format signatures
    struct TextureSignature
    {
        const char* name;
        const char* magic;  // Magic bytes to identify the format
        size_t magicSize;
    };

    // Common texture formats used in DirectX 5
    TextureSignature signatures[] = {
        {"BMP", "BM", 2},              // Bitmap file format
        {"DDS", "DDS ", 4},            // DirectDraw Surface (early versions existed)
        {"TGA", "\x00\x00\x02\x00", 4},// TGA (Uncompressed Truecolor Image)
        {"TGA (Compressed)", "\x00\x00\x10\x00", 4}, // TGA (RLE Compressed)
    };

    // Detect possible texture formats in a binary file
    void scanBinaryFile( const std::string& filePath )
    {
        std::ifstream file( filePath, std::ios::binary );
        if( !file )
        {
            std::cerr << "Error opening file: " << filePath << std::endl;
            return;
        }

        // Read entire file into a vector
        std::vector<uint8_t> data( ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>() );
        size_t fileSize = data.size();

        std::cout << "Scanning file: " << filePath << " (" << fileSize << " bytes)\n";
        std::cout << "--------------------------------------------------\n";

        // Search for known format signatures
        for( const auto& sig : signatures )
        {
            for( size_t i = 0; i <= fileSize - sig.magicSize; ++i )
            {
                if( std::memcmp( &data[i], sig.magic, sig.magicSize ) == 0 )
                {
                    std::cout << "Found " << sig.name << " at offset: " << std::hex << "0x" << i << std::dec << "\n";
                }
            }
        }

        // Check for possible texture dimensions (e.g., 256x256, 512x512)
        std::vector<int> commonSizes = { 64, 128, 256, 512, 1024, 2048 };

        for( size_t i = 0; i < fileSize - 4; i += 2 )
        {
            uint16_t width = *reinterpret_cast<uint16_t*>( &data[i] );
            uint16_t height = *reinterpret_cast<uint16_t*>( &data[i + 2] );

            if( std::find( commonSizes.begin(), commonSizes.end(), width ) != commonSizes.end() &&
                std::find( commonSizes.begin(), commonSizes.end(), height ) != commonSizes.end() )
            {
                std::cout << "Possible texture dimensions: " << width << "x" << height
                    << " at offset: " << std::hex << "0x" << i << std::dec << "\n";
            }
        }

        std::cout << "Scan completed.\n";
    }

// Structure for a standard TGA header (18 bytes)
#pragma pack(push, 1)
    struct TGAHeader
    {
        uint8_t idLength;
        uint8_t colorMapType;
        uint8_t imageType;
        uint16_t colorMapOrigin;
        uint16_t colorMapLength;
        uint8_t colorMapDepth;
        uint16_t xOrigin;
        uint16_t yOrigin;
        uint16_t width;
        uint16_t height;
        uint8_t bitsPerPixel;
        uint8_t imageDescriptor;
    };
#pragma pack(pop)

// List of known TGA headers (uncompressed & compressed)
    std::vector<std::vector<uint8_t>> tgaSignatures = {
        {0x00, 0x00, 0x02, 0x00},  // Uncompressed Truecolor TGA
        {0x00, 0x00, 0x10, 0x00}   // RLE Compressed Truecolor TGA
    };

    // Function to read a binary file into a vector
    std::vector<uint8_t> readBinaryFile( const std::string& filename )
    {
        std::ifstream file( filename, std::ios::binary );
        return std::vector<uint8_t>( ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>() );
    }

    // Function to find TGA headers inside the binary
    std::vector<size_t> findTGAHeaders( const std::vector<uint8_t>& data )
    {
        std::vector<size_t> offsets;

        for( size_t i = 0; i <= data.size() - 18; ++i )
        {
            for( const auto& sig : tgaSignatures )
            {
                if( std::memcmp( &data[i], sig.data(), sig.size() ) == 0 )
                {
                    offsets.push_back( i );
                    break;
                }
            }
        }

        return offsets;
    }

    // Function to extract and save TGAs
    void extractTGAs( const std::vector<uint8_t>& data, const std::vector<size_t>& offsets, const std::string& outputDir )
    {
        int count = 0;

        for( size_t offset : offsets )
        {
            if( offset + sizeof( TGAHeader ) > data.size() )
            {
                std::cerr << "Invalid TGA header at offset: 0x" << std::hex << offset << std::dec << "\n";
                continue;
            }

            // Read TGA header
            TGAHeader header;
            std::memcpy( &header, &data[offset], sizeof( TGAHeader ) );

            // Validate width/height
            if( header.width == 0 || header.height == 0 || header.bitsPerPixel == 0 )
            {
                std::cerr << "Skipping invalid TGA at offset: 0x" << std::hex << offset << std::dec << "\n";
                continue;
            }

            // Calculate estimated file size (header + pixel data)
            size_t pixelDataSize = ( header.width * header.height * ( header.bitsPerPixel / 8 ) );
            size_t estimatedFileSize = sizeof( TGAHeader ) + pixelDataSize;

            if( offset + estimatedFileSize > data.size() )
            {
                std::cerr << "Truncated TGA at offset: 0x" << std::hex << offset << std::dec << "\n";
                continue;
            }

            // Generate output filename
            std::string filename = outputDir + "/extracted_tga_" + std::to_string( count++ ) + ".tga";

            // Write the extracted TGA file
            std::ofstream outFile( filename, std::ios::binary );
            if( !outFile )
            {
                std::cerr << "Error writing " << filename << "\n";
                continue;
            }

            // Write TGA header and pixel data
            outFile.write( reinterpret_cast<const char*>( &data[offset] ), estimatedFileSize );
            outFile.close();

            std::cout << "Extracted TGA: " << filename
                << " (" << header.width << "x" << header.height << ", " << (int)header.bitsPerPixel << "bpp)\n";
        }
    }

    int Smain()
    {
        std::string filePath = "Assets/RAW/MapFiles/C_Dtown.cmp";  // Replace with your binary file
        std::string outputDir = "Assets/RAW/TEXTURES";  // Output directory (use "." for current directory)

        std::vector<uint8_t> data = readBinaryFile( filePath );
        if( data.empty() )
        {
            std::cerr << "Failed to read binary file!\n";
            return 1;
        }

        // Find TGA headers
        std::vector<size_t> tgaOffsets = findTGAHeaders( data );
        if( tgaOffsets.empty() )
        {
            std::cerr << "No TGA files found.\n";
            return 0;
        }

        // Extract TGAs
        extractTGAs( data, tgaOffsets, outputDir );

        return 0;
    }


    bool is_wav_header( const std::vector<uint8_t>& buffer, size_t pos )
    {
        if( pos == 0x52F531 )
        {
            //ME_ASSERT(false);
        }
        return buffer.size() >= pos + 12 &&
            buffer[pos] == 'R' && buffer[pos + 1] == 'I' && buffer[pos + 2] == 'F' && buffer[pos + 3] == 'F' &&
            buffer[pos + 8] == 'W' && buffer[pos + 9] == 'A' && buffer[pos + 10] == 'V' && buffer[pos + 11] == 'E';
    }


    uint32_t read_little_endian_uint32( const std::vector<uint8_t>& buffer, size_t pos )
    {
        return buffer[pos] | ( buffer[pos + 1] << 8 ) | ( buffer[pos + 2] << 16 ) | ( buffer[pos + 3] << 24 );
    }


    bool is_printable_char( uint8_t c )
    {
        return std::isprint( c ) || std::isspace( c );
    }


    std::string extractFirstWord( const std::string& str )
    {
        std::string firstWord;
        for( char ch : str )
        {
            if( !std::isalpha( ch ) ) break;
            firstWord += std::toupper( ch );
        }
        return firstWord;
    }


    bool isFirstWordRepeated( const std::string& str )
    {
        size_t start = str.find_first_not_of( ' ' );
        if( start == std::string::npos ) return false;

        std::string trimmed = str.substr( start );
        std::string firstWord = extractFirstWord( trimmed );
        if( firstWord.empty() ) return false; // No valid first word

        // Convert entire string to uppercase for case-insensitive search
        std::string upperStr = str;
        std::transform( upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper );

        // Find first occurrence
        size_t firstPos = upperStr.find( firstWord );
        if( firstPos == std::string::npos ) return false;

        // Search for another occurrence
        size_t secondPos = upperStr.find( firstWord, firstPos + firstWord.size() );
        return secondPos != std::string::npos;
    }


    bool writeBytesToFile( const std::vector<uint8_t>& source, size_t start, size_t end, const std::string& filename )
    {
        if( start >= end || end > source.size() )
        {
            std::cerr << "Invalid range: start=" << start << ", end=" << end << ", size=" << source.size() << std::endl;
            return false;
        }

        // Open file for binary writing
        std::ofstream outFile( filename, std::ios::binary );
        if( !outFile )
        {
            std::cerr << "Error opening file: " << filename << std::endl;
            return false;
        }

        // Write the range of bytes to the file
        outFile.write( reinterpret_cast<const char*>( &source[start] ), end - start );

        // Close file and return success
        return outFile.good();
    }


    size_t isBMPHeader( const std::vector<uint8_t>& data, size_t pos )
    {
        if( pos >= data.size() )
        {
            return std::string::npos;
        }

        if( data[pos] == 0x42 && data[pos + 1] == 0x4D )
        { // "BM" signature
            return pos; // Return the position where BMP starts
        }
        return std::string::npos; // Not found
    }


    bool extractAndWriteBMP( const std::vector<uint8_t>& data, size_t pos, const std::string& outputFile, size_t& outSize, bool writeFileToDisk = false )
    {
        size_t bmpOffset = isBMPHeader( data, pos );
        if( bmpOffset == std::string::npos )
        {
            //std::cerr << "BMP header not found!\n";
            return false;
        }

        // Read BMP headers
        BITMAPFILEHEADER fileHeader;
        BITMAPINFOHEADER infoHeader;

        if( bmpOffset + sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) > data.size() )
        {
            std::cerr << "Incomplete BMP header in data!\n";
            return false;
        }

        std::memcpy( &fileHeader, &data[bmpOffset], sizeof( fileHeader ) );
        std::memcpy( &infoHeader, &data[bmpOffset + sizeof( fileHeader )], sizeof( infoHeader ) );

        // Validate BMP signature
        if( fileHeader.bfType != 0x4D42 )
        {
            std::cerr << "Invalid BMP signature!\n";
            return false;
        }

        // Validate BMP pixel data offset
        if( fileHeader.bfOffBits >= data.size() )
        {
            //std::cerr << "Invalid pixel data offset!\n";
            return false;
        }

        // Extract pixel data
        size_t pixelDataStart = bmpOffset + fileHeader.bfOffBits;
        size_t pixelDataSize = fileHeader.bfSize - fileHeader.bfOffBits;
        if( pixelDataStart + pixelDataSize > data.size() )
        {
            std::cerr << "Potential BMP Pixel data exceeds buffer size!\n";
            return false;
        }

        if( writeFileToDisk )
        {
            // Write the extracted BMP to file
            std::ofstream outFile( outputFile, std::ios::binary );
            if( !outFile )
            {
                std::cerr << "Failed to open output file!\n";
                return false;
            }

            // Write BMP headers
            outFile.write( reinterpret_cast<char*>( &fileHeader ), sizeof( fileHeader ) );
            outFile.write( reinterpret_cast<char*>( &infoHeader ), sizeof( infoHeader ) );

            // Write pixel data
            outFile.write( reinterpret_cast<const char*>( &data[pixelDataStart] ), pixelDataSize );
            outFile.close();

            std::cout << "BMP successfully extracted to " << outputFile << "\n";
        }

        outSize = pixelDataSize + sizeof( fileHeader ) + sizeof( infoHeader );
        return true;
    }


    std::string GetCharacterID( std::string& inCharacterString )
    {
        for( size_t i = 1; i < inCharacterString.size(); ++i )
        {
            std::string prefix = inCharacterString.substr( 0, i );
            std::string upper_prefix = prefix;

            for( char& c : upper_prefix )
            {
                c = std::toupper( static_cast<unsigned char>( c ) );
            }

            if( inCharacterString.compare( i, upper_prefix.size(), upper_prefix ) == 0 )
            {
                return prefix;
            }
        }

        return inCharacterString.substr( 0, 30 );
    }


    void parse_hdz_file( const std::string& input_filename, std::vector<HeadDef>& outHeadList, std::vector<HeadDef>& outDeadHeadList, PixelImage& pixelImage )
    {
        std::ifstream input( input_filename, std::ios::binary );
        if( !input )
        {
            std::cerr << "Error: Could not open " << input_filename << "\n";
            return;
        }
        std::vector<uint8_t> buffer( ( std::istreambuf_iterator<char>( input ) ), {} );
        std::ofstream string_output( "Assets/RAW/extracted_strings.txt" );
        if( !string_output )
        {
            std::cerr << "Error: Could not create extracted_strings.txt\n";
            return;
        }

        size_t startingPos = 0;// 7359318;
        size_t pos = startingPos;
        int wav_count = 0;
        int bmp_count = 0;
        std::string wav_filename;
        size_t character_num = 0;

        std::string CharacterID;
        HeadDef* CurrentHeadDef = nullptr;
        size_t maxFileSize = 0;
        bool exportFiles = true;
        size_t lastWAV = 0;
        while( pos < buffer.size() )
        {
            if( character_num > 25 )
            {
                writeBytesToFile( buffer, startingPos, pos, "Assets/RAW/SUS.bin" );

                string_output << '[' << pos << "]" << " Last Index" << "\n";
                break;
            }
            if( is_wav_header( buffer, pos ) )
            {
                //ME_ASSERT_MSG( !CharacterID.empty(), "Parsing a wav file before we found a valid character entry.");
                uint32_t file_size = read_little_endian_uint32( buffer, pos + 4 ) + 8; // RIFF chunk size + header
                string_output << "WAV at pos=" << pos << " claims size=" << file_size << '\n';
                if( pos + file_size > buffer.size() )
                {
                    std::cerr << "Warning: Incomplete WAV file detected. Skipping.\n";
                    break;
                }
                size_t wavPos = 0;
                wavPos = pos;
                wav_filename = "Assets/RAW/AUDIO/" + std::to_string( wav_count++ ) + "_" + GetCharacterID( CurrentHeadDef->RawID ) + ".wav";
                if( exportFiles )
                {
                    std::ofstream output( wav_filename, std::ios::binary );
                    if( !output )
                    {
                        std::cerr << "Error: Could not create " << wav_filename << "\n";
                        return;
                    }

                    output.write( reinterpret_cast<const char*>( &buffer[pos] ), file_size );
                    output.close();
                    std::cout << "Extracted Audio: " << wav_filename << "\n";
                }
                string_output << '[' << wavPos << "][" << file_size << ']' << wav_filename << "\n";

                CurrentHeadDef->AssociatedAudioFiles.push_back( wav_filename );
                CharacterID = "Unknown";
                //if( pos + file_size < pixelImage.GetLength() )
                {
                    pixelImage.SetPixelRange( pos, pos + file_size-5000, PixelCategory::WAVFile );
                    if( file_size > maxFileSize )
                    {
                        maxFileSize = file_size;
                        string_output << "NEW MAXIMUM=========================" << '[' << wavPos << "][" << file_size << ']' << wav_filename << "\n";
                    }
                }
                if( lastWAV != pos )
                {
                    writeBytesToFile( buffer, lastWAV, pos, "Assets/RAW/SUS" + GetCharacterID(CurrentHeadDef->RawID) + ".bin" );
                }
                //pos += file_size;
                //continue;
                lastWAV = pos + file_size;
            }

            std::string textureOutput = std::string( "Assets/RAW/TEXTURES/" + std::to_string( bmp_count ) + ".bmp" );
            size_t size = 0;
            if( extractAndWriteBMP( buffer, pos, textureOutput, size, exportFiles ) )
            {
                //string_output << '[' << pos << "] Exported Texture: " << textureOutput << "\n";
                CurrentHeadDef->HeadPortraits.push_back( textureOutput );
                if( pos + size < pixelImage.GetLength() )
                {
                    pixelImage.SetPixelRange( pos, pos + size, PixelCategory::BMPFile );
                }
                //pos += size;
                bmp_count++;
                //continue;
            }

            // This could be a header to a head entry?
            if( buffer[pos] == 0x4A && buffer[pos + 0xB0] == 0x48 )
            {
                if( buffer[pos + 1] == 0x00 && buffer[pos + 2] == 0x00 && buffer[pos + 3] == 0x00 && buffer[pos + 4] == 0x00 && buffer[pos + 5] == 0x00 )
                {
                    string_output << '[' << pos << "]" << " Last Character" << "\n";

                    //size_t characterBinOutputEnd = ( pos + 5000 > buffer.size() ) ? buffer.size() : pos + 5000;
                    //writeBytesToFile( buffer, pos, characterBinOutputEnd, "Assets/RAW/BIN/" + std::to_string( character_num ) + ".bin" );

                    // Check for printable ASCII strings
                    size_t start = pos + 0xCB;
                    size_t end = start;
                    while( end < buffer.size() && is_printable_char( buffer[end] ) )
                    {
                        ++end;
                    }
                    if( end - start > 10 )
                    {
                        // Consider a valid string if it's at least 10 characters long
                        std::string extracted_string( buffer.begin() + start, buffer.begin() + end );
                        bool isStandardExpectation = ( std::isspace( extracted_string[0] ) && std::isupper( extracted_string[1] ) && std::islower( extracted_string[3] ) );
                        bool isDifferent = ( std::isspace( extracted_string[0] ) && std::isupper( extracted_string[1] ) && extracted_string[3] == '.' );
                        bool USSoldier = ( extracted_string.find( "U.S." ) != std::string::npos );
                        //if( ( isDifferent || isStandardExpectation ) && hasRepeatingWord )
                        //if( isStandardExpectation || isDifferent || USSoldier )
                        {
                            bool hasRepeatingWord = isFirstWordRepeated( extracted_string );
                            // Filter for a real character ID
                            std::string realName = GetCharacterID( extracted_string );
                            CharacterID = extracted_string;
                            if( !realName.empty() || hasRepeatingWord )
                            {
                                std::cout << "Found string: " << extracted_string << "\n\n";
                                //string_output << '[' << start << ']' << extracted_string << "\n";
                                HeadDef newDef;
                                newDef.RawID = extracted_string;
                                newDef.ID = ( realName.empty() && hasRepeatingWord ) ? extracted_string : realName;
                                outHeadList.push_back( std::move( newDef ) );
                                CurrentHeadDef = &outHeadList.back();
                            }
                            else
                            {
                                HeadDef newDef;
                                newDef.RawID = extracted_string;
                                newDef.ID = extracted_string;
                                outDeadHeadList.push_back( std::move( newDef ) );
                                //string_output << '[' << character_num << ']' << "NOT FOUND CHARACTER STRING BIN:" << extracted_string << "\n";
                                CurrentHeadDef = &outDeadHeadList.back();
                            }
                            CurrentHeadDef->CharacterIndex = character_num;
                        }

                        pixelImage.SetPixelRange( start, end, PixelCategory::CharacterName );
                    }
                    else
                    {
                        string_output << '[' << character_num << ']' << " CHARACTER STRING SIZE TOO SMALL\n";
                    }
                    character_num++;
                }
                else
                {
                    // just a coincidence 
                }
            }
            ++pos;
        }
        string_output.close();
        
    }


    void parse_map_file( const std::vector<Path>& inFiles )
    {
        std::ofstream string_output( "Assets/RAW/map_log.txt" );
        if( !string_output )
        {
            std::cerr << "Error: Could not create map_log.txt\n";
            return;
        }
        for( auto& path : inFiles )
        {
            std::ifstream input( path.FullPath, std::ios::binary );
            if( !input )
            {
                std::cerr << "Error: Could not open " << path.GetLocalPathString() << "\n";
                return;
            }
            std::vector<uint8_t> buffer( ( std::istreambuf_iterator<char>( input ) ), {} );


            size_t pos = 0;
            int wav_count = 0;
            int bmp_count = 0;
            std::string wav_filename;
            size_t wavPos = 0;

            std::string CharacterID;
            while( pos < buffer.size() )
            {
                if( is_wav_header( buffer, pos ) )
                {
                    //ME_ASSERT_MSG( !CharacterID.empty(), "Parsing a wav file before we found a valid character entry.");
                    uint32_t file_size = read_little_endian_uint32( buffer, pos + 4 ) + 8; // RIFF chunk size + header
                    if( pos + file_size > buffer.size() )
                    {
                        std::cerr << "Warning: Incomplete WAV file detected. Skipping.\n";
                        break;
                    }
                    wavPos = pos;
                    wav_filename = "Assets/RAW/AUDIO/" + path.GetFileNameString( false ) + "_" + std::to_string( wav_count++ ) + ".wav";
                    std::ofstream output( wav_filename, std::ios::binary );
                    if( !output )
                    {
                        std::cerr << "Error: Could not create " << wav_filename << "\n";
                        return;
                    }

                    output.write( reinterpret_cast<const char*>( &buffer[pos] ), file_size );
                    output.close();
                    std::cout << "Extracted: " << wav_filename << "\n";
                    std::cout << "Audio: " << wav_filename << "\n";
                    string_output << '[' << wavPos << ']' << wav_filename << "\n";
                    pos += file_size;
                    continue;
                }

                std::string textureOutput = std::string( "Assets/RAW/TEXTURES/" + path.GetFileNameString( false ) + "_" + std::to_string( bmp_count ) + ".bmp" );
                size_t size = 0;
                if( extractAndWriteBMP( buffer, pos, textureOutput, size ) )
                {
                    string_output << '[' << pos << "] Exported Texture: " << textureOutput << "\n";
                    bmp_count++;
                }

                ++pos;
            }
        }
        string_output.close();
    }

    struct AudioFile
    {
        std::string format;
        uint32_t offset;
        uint32_t length; // Estimated length
    };

    // Known file headers for audio formats used by the Miles Sound System
    const std::vector<std::pair<std::string, std::vector<uint8_t>>> knownHeaders = {
        {"WAV", {'R', 'I', 'F', 'F'}},  // WAV starts with RIFF, followed by "WAVE"
        {"OGG", {'O', 'g', 'g', 'S'}},  // Ogg Vorbis
        {"MP3_ID3", {'I', 'D', '3'}},   // MP3 with ID3 tag
        {"MP3_MPEG", {0xFF, 0xFB}},     // MPEG Layer 3 (MP3)
        {"MP3_MPEG", {0xFF, 0xF3}},     // MPEG Layer 3 (MP3)
        {"MIDI", {'M', 'T', 'h', 'd'}}, // MIDI files
        {"MSS", {'M', 'S', 'S'}},       // Possible Miles Sound System format
        {"AGF", {'A', 'G', 'F', ' '}}   // Miles Advanced Game Format (tentative)
    };
    const std::vector<std::pair<std::string, std::vector<uint8_t>>> audioHeaders = {
    {"WAV", {'R', 'I', 'F', 'F'}},
    {"MSS", {'M', 'S', 'S', ' '}},
    {"AGF", {'A', 'G', 'F', ' '}},
    {"XMI", {'M', 'T', 'h', 'd'}},
    };

// Valid MP3 bitrates for MPEG-1 Layer III (index 1-14 valid)
    const uint32_t mp3Bitrates[] = { 0, 32000, 40000, 48000, 56000, 64000, 80000, 96000,
                                   112000, 128000, 160000, 192000, 224000, 256000, 320000 };

    // Valid MP3 sample rates (MPEG-1: index 0-2 valid)
    const uint32_t mp3SampleRates[] = { 44100, 48000, 32000, 0 };

    // Function to validate MP3 headers
    bool isValidMP3Header( const std::vector<uint8_t>& buffer, size_t pos )
    {
        if( pos + 4 >= buffer.size() ) return false;

        uint32_t header = ( buffer[pos] << 24 ) | ( buffer[pos + 1] << 16 ) |
            ( buffer[pos + 2] << 8 ) | buffer[pos + 3];

// Sync word should be 0xFFE (11 bits set)
        if( ( header & 0xFFE00000 ) != 0xFFE00000 ) return false;

        // MPEG version (must be 1, 2, or 2.5)
        uint8_t mpegVersion = ( header >> 19 ) & 0x3;
        if( mpegVersion == 1 ) return false; // Invalid

        // Layer (must be 1, 2, or 3)
        uint8_t layer = ( header >> 17 ) & 0x3;
        if( layer != 1 ) return false; // Must be Layer III

        // Bitrate index (must be valid)
        uint8_t bitrateIndex = ( header >> 12 ) & 0xF;
        if( bitrateIndex == 0 || bitrateIndex == 15 ) return false; // Invalid

        // Sample rate index (must be valid)
        uint8_t sampleRateIndex = ( header >> 10 ) & 0x3;
        if( sampleRateIndex == 3 ) return false; // Invalid

        return true;
    }
    // Function to search for known audio headers in a binary file
    std::vector<AudioFile> scanForAudioHeaders( const std::vector<uint8_t>& buffer )
    {
        std::vector<AudioFile> foundFiles;

        for( size_t i = 0; i < buffer.size(); ++i )
        {
            for( const auto& [format, signature] : knownHeaders )
            {
                if( i + signature.size() <= buffer.size() &&
                    std::equal( signature.begin(), signature.end(), buffer.begin() + i ) )
                {

                    uint32_t estimatedLength = 0;

                    if( format == "WAV" && i + 8 < buffer.size() )
                    {
                        estimatedLength = *reinterpret_cast<const uint32_t*>( &buffer[i + 4] ) + 8;
                    }
                    else if( format == "MP3_MPEG" || format == "MP3_ID3" )
                    {
                        if( !isValidMP3Header( buffer, i ) ) continue;
                        estimatedLength = 512 * 1024; // Rough limit for MP3 extraction
                    }

                    foundFiles.push_back( { format, static_cast<uint32_t>( i ), estimatedLength } );
                    BRUH_FMT( "%s: %i", format.c_str(), i );
                }
            }
        }

        return foundFiles;
    }

    void extractAudioFiles( const std::string& filename, const std::vector<uint8_t>& buffer, const std::vector<AudioFile>& files )
    {
        for( size_t i = 0; i < files.size(); ++i )
        {
            std::string outputFilename = "Assets/RAW/AUDIO/" + filename + "_" + files[i].format + "_" + std::to_string( i ) + "." +
                ( files[i].format == "WAV" ? "wav" : "mp3" );

            uint32_t end = std::min( files[i].offset + files[i].length, static_cast<uint32_t>( buffer.size() ) );

            std::ofstream outFile( outputFilename, std::ios::binary );
            if( !outFile )
            {
                std::cerr << "Failed to write file: " << outputFilename << std::endl;
                continue;
            }

            outFile.write( reinterpret_cast<const char*>( &buffer[files[i].offset] ), end - files[i].offset );
            outFile.close();

            std::cout << "Extracted " << files[i].format << " to " << outputFilename << " (" << end - files[i].offset << " bytes)" << std::endl;
        }
    }

    // Function to find known audio headers in binary data
    std::vector<uint32_t> findAudioOffsets( const std::vector<uint8_t>& buffer )
    {
        std::vector<uint32_t> offsets;
        for( size_t i = 0; i < buffer.size() - 4; ++i )
        {
            for( const auto& [format, signature] : audioHeaders )
            {
                if( std::equal( signature.begin(), signature.end(), buffer.begin() + i ) )
                {
                    offsets.push_back( i );
                }
            }
        }
        return offsets;
    }

    // Function to detect structured ADPCM-like patterns
    std::vector<uint32_t> findADPCMBlocks( const std::vector<uint8_t>& buffer )
    {
        std::vector<uint32_t> offsets;

        for( size_t i = 0; i < buffer.size() - 512; i += 512 )
        { // Step in chunks of 512 bytes
            bool structured = true;

            // Check for repeating low-entropy patterns (common in ADPCM)
            for( size_t j = 0; j < 512; ++j )
            {
                if( buffer[i + j] > 127 && buffer[i + j] < 255 )
                {
                    structured = false;
                    break;
                }
            }

            if( structured ) offsets.push_back( i );
        }
        return offsets;
    }

    // Extract detected audio files
    void extractAudioFilesNEW( const std::string& filename, const std::vector<uint8_t>& buffer, const std::vector<uint32_t>& offsets, const std::string& format )
    {
        for( size_t i = 0; i < offsets.size(); ++i )
        {
            uint32_t start = offsets[i];
            uint32_t end = ( i + 1 < offsets.size() ) ? offsets[i + 1] : buffer.size();

            std::string outputFilename = "Assets/RAW/AUDIO/" + filename + "_extracted_" + format + "_" + std::to_string( i ) + ".dat";
            std::ofstream outFile( outputFilename, std::ios::binary );
            if( !outFile )
            {
                std::cerr << "Failed to write file: " << outputFilename << std::endl;
                continue;
            }

            outFile.write( reinterpret_cast<const char*>( &buffer[start] ), end - start );
            outFile.close();
            std::cout << "Extracted " << format << " audio: " << outputFilename << " (" << ( end - start ) << " bytes)\n";
        }
    }
    int scanAndExtractAudioFiles( Path& inPath )
    {
        std::ifstream file( inPath.FullPath, std::ios::binary );
        if( !file )
        {
            std::cerr << "Error opening file: " << inPath.FullPath << std::endl;
            return 1;
        }

        std::vector<uint8_t> buffer( std::istreambuf_iterator<char>( file ), {} );

        auto headerOffsets = findAudioOffsets( buffer );
        extractAudioFilesNEW( inPath.GetFileNameString( false ), buffer, headerOffsets, "known" );

        // Find potential ADPCM blocks
        auto adpcmOffsets = findADPCMBlocks( buffer );
        extractAudioFilesNEW( inPath.GetFileNameString( false ), buffer, adpcmOffsets, "adpcm" );

        return 0;
    }
}