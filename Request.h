#pragma once
#include <iostream>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
int PerformHttpRequest(const std::string& url, htmlDocPtr response);