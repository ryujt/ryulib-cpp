#include <iostream>
#include <ctime>
#include <ryulib/MemoryBuffer.hpp>

int main()
{
    MemoryBuffer memoryBuffer;

    int buffer_size = 127;
    void *buffer = malloc(buffer_size);

    srand(time(0));

    while (true)
    {
        int data_size = (rand() % 1024) + 128;
        memoryBuffer.write(buffer, buffer_size);
        char *src = (char *)memoryBuffer.read(data_size);
        if (src != nullptr)
        {
            int count = 0;
            for (int i = 0; i < data_size; i++)
                count = count + *src;
            printf("data_size: %8d, count: %d \r", data_size, count);
            free(src);
        }
    }
}
