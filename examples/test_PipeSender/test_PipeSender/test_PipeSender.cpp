#include <iostream>
#include <ryulib/PipeSender.hpp>

int main()
{
    PipeSender pipe;
    pipe.setOnConnected([](const void* sender) {
        printf("connected \n");
    });
    pipe.open("\\\\.\\pipe\\test");

    while (true) {

    }
}
