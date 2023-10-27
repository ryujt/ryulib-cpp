#pragma once

#include <lzma.h>

class LZMA {
public:
    static bool encode(void* inputData, int inputSize, void* outputData, int& outputSize)
    {
        uint8_t* in = (uint8_t*)inputData;
        uint8_t* out = (uint8_t*)outputData;
        size_t out_pos = 0;

        lzma_ret ret = lzma_easy_buffer_encode(LZMA_PRESET_DEFAULT, LZMA_CHECK_CRC64, nullptr,
            in, inputSize,
            out, &out_pos, outputSize);

        if (ret != LZMA_OK)
        {
            return false;
        }

        outputSize = out_pos;
        return true;
    }

    static bool decode(void* inputData, int inputSize, void* outputData, int& outputSize)
    {
        uint8_t* in = (uint8_t*)inputData;
        uint8_t* out = (uint8_t*)outputData;
        size_t in_pos = 0, out_pos = 0;
        uint64_t memlimit = UINT64_MAX;

        lzma_ret ret = lzma_stream_buffer_decode(&memlimit, 0, nullptr,
            in, &in_pos, inputSize,
            out, &out_pos, outputSize);

        if (ret != LZMA_OK)
        {
            return false;
        }

        outputSize = out_pos;
        return true;
    }
};