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

    bool is_wav_header( const std::vector<uint8_t>& buffer, size_t pos )
    {
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
        if( data[pos] == 0x42 && data[pos + 1] == 0x4D )
        { // "BM" signature
            return pos; // Return the position where BMP starts
        }
        return std::string::npos; // Not found
    }


    bool extractAndWriteBMP( const std::vector<uint8_t>& data, size_t pos, const std::string& outputFile )
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
            std::cerr << "Pixel data exceeds buffer size!\n";
            return false;
        }

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

        return "";
    }


    void parse_hdz_file( const std::string& input_filename, std::vector<HeadDef>& outHeadList, std::vector<HeadDef>& outDeadHeadList )
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

        size_t pos = 0;
        int wav_count = 0;
        int bmp_count = 0;
        std::string wav_filename;
        size_t wavPos = 0;
        size_t character_num = 0;

        std::string CharacterID;
        HeadDef* CurrentHeadDef = nullptr;
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
                wav_filename = "Assets/RAW/AUDIO/" + std::to_string( wav_count++ ) + "_" + GetCharacterID( CurrentHeadDef->RawID ) + ".wav";
                std::ofstream output( wav_filename, std::ios::binary );
                if( !output )
                {
                    std::cerr << "Error: Could not create " << wav_filename << "\n";
                    return;
                }
                CurrentHeadDef->AssociatedAudioFiles.push_back( wav_filename );

                output.write( reinterpret_cast<const char*>( &buffer[pos] ), file_size );
                output.close();
                std::cout << "Extracted: " << wav_filename << "\n";
                std::cout << "Audio: " << wav_filename << "\n";
                string_output << '[' << wavPos << ']' << wav_filename << "\n";
                CharacterID = "Unknown";
                pos += file_size;
            }

            std::string textureOutput = std::string( "Assets/RAW/TEXTURES/" + std::to_string( bmp_count ) + ".bmp" );
            if( extractAndWriteBMP( buffer, pos, textureOutput ) )
            {
                string_output << '[' << pos << "] Exported Texture: " << textureOutput << "\n";
                CurrentHeadDef->HeadPortraits.push_back( textureOutput );
                bmp_count++;
            }

            // This could be a header to a head entry?
            if( buffer[pos] == 0x4A && buffer[pos + 0xB0] == 0x48 )
            {
                if( buffer[pos + 1] == 0x00 && buffer[pos + 2] == 0x00 && buffer[pos + 3] == 0x00 && buffer[pos + 4] == 0x00 && buffer[pos + 5] == 0x00 )
                {
                    size_t characterBinOutputEnd = ( pos + 5000 > buffer.size() ) ? buffer.size() : pos + 5000;
                    writeBytesToFile( buffer, pos, characterBinOutputEnd, "Assets/RAW/BIN/" + std::to_string( character_num ) + ".bin" );

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
                                string_output << '[' << start << ']' << extracted_string << "\n";
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
                                string_output << '[' << character_num << ']' << "NOT FOUND CHARACTER STRING BIN:" << extracted_string << "\n";
                                CurrentHeadDef = &outDeadHeadList.back();
                            }
                            CurrentHeadDef->CharacterIndex = character_num;
                        }
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


    void parse_map_file( const Path& inMapPath )
    {
        std::ifstream input( inMapPath.FullPath, std::ios::binary );
        if( !input )
        {
            std::cerr << "Error: Could not open " << inMapPath.GetLocalPathString() << "\n";
            return;
        }
        std::vector<uint8_t> buffer( ( std::istreambuf_iterator<char>( input ) ), {} );
        std::ofstream string_output( "Assets/RAW/map_log.txt" );
        if( !string_output )
        {
            std::cerr << "Error: Could not create map_log.txt\n";
            return;
        }

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
                wav_filename = "Assets/RAW/AUDIO/" + inMapPath.GetFileNameString( false ) + "_" + std::to_string( wav_count++ ) + ".wav";
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
            }

            std::string textureOutput = std::string( "Assets/RAW/TEXTURES/" + inMapPath.GetFileNameString( false ) + "_" + std::to_string( bmp_count ) + ".bmp" );
            if( extractAndWriteBMP( buffer, pos, textureOutput ) )
            {
                string_output << '[' << pos << "] Exported Texture: " << textureOutput << "\n";
                bmp_count++;
            }

            ++pos;
        }
        string_output.close();
    }
}