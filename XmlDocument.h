#pragma once

#include <iostream>
#include <map>
#include <vector>

struct XmlNode
{
    enum Type
    {
        NotSupported,
        Element
    };

    Type type;
    std::string name;
    std::map<std::string, std::string> attributes;
    std::vector<XmlNode> children;

    void print(std::ostream & stream = std::cout, int depth = 0) const;
};

class XmlDocument
{
public:
    XmlDocument() { }

    /*
     * @return true on success, false on error
     * @see getLastError
     */
    bool load(const char * filename);

    /*
     * @param data zero-terminated entire xml document contents.
     */
    bool parse(const char * data);

    const char * getLastError() const;

    void print(std::ostream & stream = std::cout) const;

    const XmlNode & getRoot() const;

private:
    void skipWhitespaces(const char * & data);
    bool parseNode(const char * & data, XmlNode & node);
    bool parseNodeContents(const char * & data, XmlNode & node);
    bool parseName(const char * & data, std::string & name);
    bool parseAttributes(const char *& data, std::map<std::string, std::string> & attr);

private:
    XmlNode root;
    const char * error = "";
};
