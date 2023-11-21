#include <iostream>
#include <ryulib/HttpDownloader.hpp>

int main()
{
    HttpDownloader downloader;

    downloader.setOnData([&](const void* data, int size, int total) {
        printf("HttpDownloader.do_work - size: %d, sum: %d \n", size, total);
    });
        
    HttpDownloader::init();
    downloader.download("http://dokdo.mofa.go.kr/m/kor/img/main/picSlide3.jpg");

    std::cout << "download ended! \n";
}
