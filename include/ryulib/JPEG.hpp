#pragma once

#include <mutex>
#include <turbojpeg.h>

class JPEG {
public:
    static std::once_flag initFlag;
    static tjhandle jpegCompressor;
    static tjhandle jpegDecompressor;

    static bool encode(void* inputData, int width, int height, void* outputData, unsigned long& outputSize)
    {
        std::call_once(initFlag, []() {
            jpegCompressor = tjInitCompress();
            jpegDecompressor = tjInitDecompress();
            });

        if (tjCompress2(jpegCompressor, (unsigned char*)inputData, width, 0, height, TJPF_RGBA, (unsigned char**)&outputData, &outputSize, TJSAMP_444, 80, TJFLAG_FASTDCT) == -1) {
            return false;
        }
        return true;
    }

    static bool decode(void* inputData, unsigned long inputSize, void* outputData, int width, int height)
    {
        std::call_once(initFlag, []() {
            jpegCompressor = tjInitCompress();
            jpegDecompressor = tjInitDecompress();
            });

        if (tjDecompress2(jpegDecompressor, (unsigned char*)inputData, inputSize, (unsigned char*)outputData, width, 0, height, TJPF_RGBA, 0) == -1) {
            return false;
        }

        return true;
    }
};

std::once_flag JPEG::initFlag;
tjhandle JPEG::jpegCompressor = nullptr;
tjhandle JPEG::jpegDecompressor = nullptr;
