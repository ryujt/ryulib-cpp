#pragma once

#include <string>
#include <functional>
#include <curl/curl.h>
#include <curl/easy.h>

using namespace std;

// [](const void* data, int size, int total) data, size, total
typedef function<void(const void*, int, int)> HttpDownloaderDataEvent; 

class HttpDownloader {
public:
	static void init() {
		curl_global_init(CURL_GLOBAL_ALL);
	}

	bool download(string url) {
		needToStop = false;
		total = 0;
		CURL* handle = curl_easy_init();
		curl_easy_setopt(handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(handle, CURLOPT_URL, url.c_str());

		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, callback_download);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*) this);

		curl_easy_setopt(handle, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, callback_progress);
		curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, (void*) this);
		CURLcode res = curl_easy_perform(handle);
		curl_easy_cleanup(handle);
		return res == CURLE_OK;
	}

	void stop() {
		needToStop = true;
	}

	void setOnData(HttpDownloaderDataEvent event) { onData = event; }

private:
	static size_t callback_download(void* contents, size_t size, size_t nmemb, void* userp)
	{
		size_t realsize = size * nmemb;
		HttpDownloader* downloader = (HttpDownloader*) userp;
		downloader->do_work(contents, realsize);
		return realsize;
	}

	static size_t callback_progress(void* userp, double dltotal, double dlnow, double ultotal, double ulnow)
	{
		HttpDownloader* downloader = (HttpDownloader*) userp;
		if (downloader->needToStop) return -1;
		return 0;
	}

	void do_work(const void* data, int size) {
		total = total + size;
		if (onData != nullptr) onData(data, size, total);
	}

	int total = 0;
	bool needToStop = false;

	HttpDownloaderDataEvent onData = nullptr;
};
