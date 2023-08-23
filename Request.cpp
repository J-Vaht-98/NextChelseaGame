//#include <iostream>
//#include <curl/curl.h>
//
//size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
//    size_t total_size = size * nmemb;
//    std::string* output = static_cast<std::string*>(userp);
//    output->append(static_cast<char*>(contents), total_size);
//    return total_size;
//}
//int PerformHttpRequest(const std::string& url, std::string& response) {
//    CURL* curl = curl_easy_init();
//    if (!curl) {
//        std::cerr << "Failed to initialize cURL" << std::endl;
//        return -1;
//    }
//
//    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
//
//    CURLcode res = curl_easy_perform(curl);
//    if (res != CURLE_OK) {
//        std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
//        return -1;
//    }
//
//
//
//    curl_easy_cleanup(curl);
//    return 0;
//}
