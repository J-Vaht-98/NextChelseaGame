#include <iostream>
#include <libxml/HTMLparser.h>

using namespace std;

xmlNode* FindElementById(xmlNode* node, const char* id) {
    for (xmlNode* curr = node; curr != nullptr; curr = curr->next) {
        if (curr->type == XML_ELEMENT_NODE) {
            const char* elemId = reinterpret_cast<const char*>(xmlGetProp(curr, reinterpret_cast<const xmlChar*>("id")));
            if (elemId && strcmp(elemId, id) == 0) {
                return curr;
            }
        }

        xmlNode* childResult = FindElementById(curr->children, id);
        if (childResult) {
            return childResult;
        }
    }

    return nullptr;
}

xmlNode* ParseHtmlAndFindElement(const std::string& html, const char* id) {
    htmlDocPtr doc = htmlReadMemory(html.c_str(), html.length(), nullptr, nullptr, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc) {
        std::cerr << "Failed to parse HTML" << std::endl;
        return nullptr;
    }

    xmlNode* root = xmlDocGetRootElement(doc);
    xmlNode* result = FindElementById(root, id);

    xmlFreeDoc(doc);
    return result;
}