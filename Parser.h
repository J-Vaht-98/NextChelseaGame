#pragma once
#include <iostream>
#include <libxml/HTMLparser.h>

using namespace std;

xmlNode* FindElementById(xmlNode* node, const char* id);
xmlNode* ParseHtmlAndFindElement(const string& html, const char* id);