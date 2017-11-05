#include <fstream>
#include <vector>

#include "utils.h"


void loadBmp(const char* filepath, std::vector<unsigned char>* outPixels, int* outWidth, int* outHeight)
{
    std::ifstream hFile(filepath, std::ios::in | std::ios::binary);
    if (!hFile.is_open()) throw std::invalid_argument("Error: File Not Found.");

    hFile.seekg(0, std::ios::end);
    std::streampos length = hFile.tellg();
    hFile.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> fileInfo(length);
    hFile.read(reinterpret_cast<char*>(fileInfo.data()), 54);

    if (fileInfo[0] != 'B' && fileInfo[1] != 'M')
    {
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. Bitmap Required.");
    }

    if (fileInfo[28] != 24 && fileInfo[28] != 32)
    {
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. 24 or 32 bit Image Required.");
    }

    short bitsPerPixel = fileInfo[28];
    *outWidth = fileInfo[18] + (fileInfo[19] << 8);
    *outHeight = fileInfo[22] + (fileInfo[23] << 8);
    std::uint32_t pixelsOffset = fileInfo[10] + (fileInfo[11] << 8);
    std::uint32_t size = ((*outWidth * bitsPerPixel + 31) / 32) * 4 * (*outHeight);
    outPixels->resize(size);

    hFile.seekg(pixelsOffset, std::ios::beg);
    hFile.read(reinterpret_cast<char*>(outPixels->data()), size);
    hFile.close();
}
