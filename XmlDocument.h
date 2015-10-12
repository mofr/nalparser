#pragma once

#include <ostream>
#include <map>
#include <vector>

struct XmlNode
{
    enum Type
    {
        NotSupported,
        Element
    };

    std::string name;
    std::map<std::string, std::string> attr;
    std::vector<XmlNode> children;
    Type type;

    void print(std::ostream & stream, int depth = 0) const;
};

class XmlDocument
{
public:
    XmlDocument() { }

    bool load(const char * filename);
    bool parse(const char * data);
    const char * getLastError() const;
    void print(std::ostream & stream) const;

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
