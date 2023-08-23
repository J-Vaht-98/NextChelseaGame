#include <iostream>
#include <curl/curl.h>
#include "Request.h"
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>
using namespace std;
using json = nlohmann::json;
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t total_size = size * nmemb;
	std::string* output = static_cast<std::string*>(userp);
	output->append(static_cast<char*>(contents), total_size);
	return total_size;
}
char* findSiteData(xmlNodePtr root) {
	for (xmlNodePtr curr = root; curr != nullptr; curr = curr->next) {
		if (curr->type == XML_ELEMENT_NODE && strcmp((const char*)curr->name, "div") == 0) {
			xmlChar* find = xmlCharStrdup("data-component");
			xmlChar* prop = xmlGetProp(curr, find);
			xmlChar* siteHeader = xmlCharStrdup("SiteHeader");
			if (prop && xmlStrEqual(prop, siteHeader)) {
				xmlChar* dataPropsAttribute = xmlCharStrdup("data-props");
				xmlChar* serializedJSON = xmlGetProp(curr, dataPropsAttribute);
				//cout << serializedJSON << endl;

				xmlFree(find);
				xmlFree(prop);
				xmlFree(siteHeader);
				xmlFree(dataPropsAttribute);

				char* result = _strdup(reinterpret_cast<char*>(serializedJSON));
				xmlFree(serializedJSON);
				return result;
			}
			xmlFree(find);
			xmlFree(prop);
			xmlFree(siteHeader);
		}
		char* result = findSiteData(curr->children);
		if (result) {
			return result;
		}
	}
	return nullptr;
}
void printKeys(const json& jsonObject, const std::string& parentKey = "") {
	for (json::const_iterator it = jsonObject.begin(); it != jsonObject.end(); ++it) {
		std::string currentKey = parentKey.empty() ? it.key() : (parentKey + "." + it.key());
		std::cout << "Key: " << currentKey << std::endl;

		if (it.value().is_object()) {
			printKeys(it.value(), currentKey); // Recursively print subkeys
		}
	}
}
bool isLeapYear(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int dayOfYear(int month, int day, int year) {
	const int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	int totalDays = day;

	for (int i = 0; i < month; ++i) {
		totalDays += daysInMonth[i];
	}

	if (month > 1 && isLeapYear(year)) {
		totalDays += 1;
	}

	return totalDays;
}
tm* getGameTime(string& kickoffDate, string& kickoffTime) {

	// Get the current time point
	auto currentTime = std::chrono::system_clock::now();

	// Convert the time point to a time_t object
	std::time_t time = std::chrono::system_clock::to_time_t(currentTime);

	// Convert time_t to a local time struct using localtime_s
	struct tm localTime;
	localtime_s(&localTime, &time);
	tm* gameTime = new tm;
	if (!gameTime) {
		std::cerr << "Couldnt allocate time" << endl;
		return nullptr;
	}
	int i = 0;
	enum DateTimePart {
		WEEKDAY = 0,
		DAY = 1,
		MONTH = 2,
		YEAR = 3,
		HOUR = 4,
		MINUTE = 5
	};
	std::unordered_map<std::string, int> months, days;

	months["Jan"] = 0;
	months["Feb"] = 1;
	months["Mar"] = 2;
	months["Apr"] = 3;
	months["May"] = 4;
	months["Jun"] = 5;
	months["Jul"] = 6;
	months["Aug"] = 7;
	months["Sep"] = 8;
	months["Oct"] = 9;
	months["Nov"] = 10;
	months["Dec"] = 11;
	days["Mon"] = 0;
	days["Tue"] = 1;
	days["Wed"] = 2;
	days["Thu"] = 3;
	days["Fri"] = 4;
	days["Sat"] = 5;
	days["Sun"] = 6;

	int cIndex = 0;
	string date[] = { "","","","","","" };
	string dateTime = kickoffDate + ' ' + kickoffTime;
	while (dateTime[i] != '\0') {
		char c = dateTime[i];

		if (c == ' ' || c == ':') {
			cIndex++;
			i++;
			continue;
		}
		switch (cIndex) {
		case WEEKDAY:
			date[WEEKDAY] += c;
			break;
		case DAY:
			date[DAY] += c;
			break;
		case MONTH:
			date[MONTH] += c;
			break;
		case YEAR:
			date[YEAR] += c;
			break;
		case HOUR:
			date[HOUR] += c;
			break;
		case MINUTE:
			date[MINUTE] += c;
			break;
		default:
			break;
		}
		i++;
	}

	int month = months[date[MONTH]];
	int day = std::stoi(date[DAY]);
	int weekday = days[date[WEEKDAY]];
	int year = std::stoi(date[YEAR]);
	int hour = std::stoi(date[HOUR]);
	int minutes = std::stoi(date[MINUTE]);

	gameTime->tm_sec = 0;
	gameTime->tm_min = minutes;
	gameTime->tm_hour = hour + 2; //shitty way to convert to correct timezone. TODO:do better
	gameTime->tm_mday = day;
	gameTime->tm_mon = month;
	gameTime->tm_year = (year - 1900);
	gameTime->tm_wday = weekday;
	gameTime->tm_yday = dayOfYear(month, day, year);
	gameTime->tm_isdst = localTime.tm_isdst;


	return gameTime;
}
std::string formatTimeDifference(tm* futureTime) {
	time_t currentTime;
	time(&currentTime);

	double diffSeconds = difftime(mktime(futureTime), currentTime);

	int days = static_cast<int>(diffSeconds / (24 * 60 * 60));
	diffSeconds -= days * 24 * 60 * 60;

	int hours = static_cast<int>(diffSeconds / (60 * 60));
	diffSeconds -= hours * 60 * 60;

	int minutes = static_cast<int>(diffSeconds / 60);
	diffSeconds -= minutes * 60;

	int seconds = static_cast<int>(diffSeconds);

	std::ostringstream oss;
	if (days > 0) {
		oss << days << " day" << (days > 1 ? "s" : "");
		if (hours > 0 || minutes > 0) {
			oss << ", ";
		}
	}

	if (hours > 0) {
		oss << hours << " hour" << (hours > 1 ? "s" : "");
		if (minutes > 0) {
			oss << ", ";
		}
	}

	if (minutes > 0) {
		oss << minutes << " minute" << (minutes > 1 ? "s" : "");
	}
	if (seconds > 0) {
		oss << ", ";
		oss << seconds << " second" << (minutes > 1 ? "s" : "");
	}

	return oss.str();
}

int main(int argc, char* argv[])
{
	//Make http request to retrieve latest game
	const char* url = "https://www.chelseafc.com/en/matches/mens-fixtures-and-results";
	const char* encoding = "utf-8";
	string response;

	CURL* curl = curl_easy_init();
	if (!curl) {
		std::cerr << "Failed to initialize cURL" << std::endl;
		return -1;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
		return -1;
	}
	curl_easy_cleanup(curl);




	//Convert response to html

	htmlDocPtr html = htmlReadMemory(response.c_str(), response.length(), nullptr, nullptr, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
	xmlNodePtr root = xmlDocGetRootElement(html);


	//Incoming html uses a serialized json string in an attribute called data-props. <div data-component="SiteHeader" data-props=.... >

	//find the div element that has attribute data-component="SiteHeader"
	// Get the value of the data-props attribute of that element

	char* jsonString = findSiteData(root);

	if (!jsonString) {
		std::cout << "Couldnt find unserialized json from received HTML " << endl;
	}

	//Extract the team names and date
	string kickoffDate, competition, kickoffTime, homeClubName, awayClubName;
	try {

		//siteLinks.promo.componentProps.matchUp
		json jsonObject = json::parse(jsonString);
		int siteLinkNr = 2; //might change
		kickoffDate = jsonObject["siteLinks"][siteLinkNr]["promo"]["componentProps"]["kickoffDate"];
		competition = jsonObject["siteLinks"][siteLinkNr]["promo"]["componentProps"]["competition"];
		kickoffTime = jsonObject["siteLinks"][siteLinkNr]["promo"]["componentProps"]["kickoffTime"];
		homeClubName = jsonObject["siteLinks"][siteLinkNr]["promo"]["componentProps"]["matchUp"]["home"]["clubName"];
		awayClubName = jsonObject["siteLinks"][siteLinkNr]["promo"]["componentProps"]["matchUp"]["away"]["clubName"];
		//cout << jsonObject["siteLinks"]<< endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Error parsing JSON: " << e.what() << std::endl;
		return -1;
	}


	tm* gameTime = getGameTime(kickoffDate, kickoffTime);

	string timeRemaining = formatTimeDifference(gameTime);

	//IDEA: if timeremaining < 1hr, find the match centre link from the json and get lineups
	std::cout << "Press ENTER to exit..." << endl;
	std::cout << "\nThe next game is on " << kickoffDate << " @ " << kickoffTime << " (BST) " << endl << homeClubName << " (H) vs " << awayClubName << " (A)" << endl;

	// Create a separate thread to listen for a key press
	std::atomic<bool> keyPressed(false);
	std::thread keyListener([&]() {
		std::cin.get(); // Wait for any key press
		keyPressed = true;
		std::cout << "Exiting..." << endl;
	});

	while (!keyPressed) {
		std::string formattedDifference = formatTimeDifference(gameTime);
		std::cout << "Time until kickoff: " << formattedDifference << " \r"  << std::flush;

		// Wait for one second
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	cout << endl << endl;
	keyListener.join();
	return 0;
}
/*
Ideas:
	Timer that ticks down.
		Remove the carriage return thing cause its hacky
	Have lineup data (released about 1hr before game)
	GUI -- the webscraped data has links to various images and stuff


*/