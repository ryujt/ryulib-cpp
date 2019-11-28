#include <ryulib/SocketClient.hpp>

int main()
{
	SocketClient socket;
	socket.connect("100.100.100.100", 1234);
    
	while (true) {
		Sleep(1000);
	}

}
