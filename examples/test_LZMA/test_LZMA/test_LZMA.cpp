#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <ryulib/LZMA.hpp>

int main()
{
    const int inputSize = 1024 * 1024;
    uint8_t* inputData = new uint8_t[inputSize];
    for (int i = 0; i < inputSize; ++i)
    {
        inputData[i] = rand() % 16;
    }

    int compressedSize = inputSize * 2;
    uint8_t* compressedData = new uint8_t[compressedSize];

    bool encodeSuccess = LZMA::encode(inputData, inputSize, compressedData, compressedSize);
    if (encodeSuccess)
    {
        std::cout << "Encoded size: " << compressedSize << std::endl;
    }
    else
    {
        std::cout << "Encoding failed." << std::endl;
    }

    int decompressedSize = inputSize;
    uint8_t* decompressedData = new uint8_t[decompressedSize];

    bool decodeSuccess = LZMA::decode(compressedData, compressedSize, decompressedData, decompressedSize);
    if (decodeSuccess)
    {
        std::cout << "Decoded size: " << decompressedSize << std::endl;
    }
    else
    {
        std::cout << "Decoding failed." << std::endl;
    }

    if (memcmp(inputData, decompressedData, inputSize) == 0)
    {
        std::cout << "Memory blocks are identical!" << std::endl;
    }
    else
    {
        std::cout << "Memory blocks are different!" << std::endl;
    }

    delete[] inputData;
    delete[] compressedData;
    delete[] decompressedData;

    return 0;
}
