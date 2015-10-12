#include "XmlDocument.h"
#include <fstream>

void XmlNode::print(std::ostream & stream, int depth) const
{
    for(int i = 0; i < depth; ++i)
    {
        stream << "  ";
    }

    stream << name;
    for(const auto & item : attr)
    {
        stream << " " << item.first << "=" << item.second;
    }
    stream << std::endl;
    for(const auto & child : children)
    {
        child.print(stream, depth + 1);
    }
}

bool XmlDocument::load(const char * filename)
{
    error = "";

    std::ifstream file(filename);
    if(!file.is_open())
    {
        error = "Cant' open file";
        return false;
    }
    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    data.resize(data.size() + 1, 0);
    return parse(data.data());
}

bool XmlDocument::parse(const char * data)
{
    error = "";

    while(*data != 0)
    {
        if(*data == '<')
        {
            XmlNode node;
            if(!parseNode(data, node))
            {
                return false;
            }
            if(node.type == XmlNode::Element)
            {
                root = node;
            }
        }
        ++data;
    }

    return true;
}

const char *XmlDocument::getLastError() const
{
    return error;
}

void XmlDocument::print(std::ostream & stream) const
{
    root.print(stream);
}

const XmlNode &XmlDocument::getRoot() const
{
    return root;
}

void XmlDocument::skipWhitespaces(const char * & data)
{
    while(*data == ' ' || *data == '\t' || *data == '\n' || *data == '\r')
    {
        ++data;
    }
}

bool XmlDocument::parseNode(const char * & data, XmlNode & node)
{
    ++data;
    if(*data == '?' || *data == '!')
    {
        while(*data != '>')
        {
            if(*data == 0)
            {
                error = "Unexpected end of file";
                return false;
            }
            ++data;
        }
        node.type = XmlNode::NotSupported;
    }
    else
    {
        node.type = XmlNode::Element;
        if(!parseName(data, node.name))
        {
            return false;
        }

        if(!parseAttributes(data, node.attr))
        {
            return false;
        }

        if(data[0] == '/' && data[1] == '>')
        {
            return true;
        }

        if(!parseNodeContents(data, node))
        {
            return false;
        }
    }
    return true;
}

bool XmlDocument::parseNodeContents(const char * & data, XmlNode & node)
{
    while (*data != 0)
    {
        if (*data == '<')
        {
            if (data[1] == '/')
            {
                break;
            }
            else
            {
                if(data[1] == 0)
                {
                    error = "Unexpected end of file";
                    return false;
                }
                XmlNode child;
                if(!parseNode(data, child))
                {
                    return false;
                }
                node.children.push_back(child);
            }
        }
        ++data;
    }
    return true;
}

bool XmlDocument::parseName(const char * & data, std::string & name)
{
    const char *nameBegin = data;
    while (*data != ' ' && *data != '>' && *data != '/' && *data != '>' && *data != '=' && *data != '!' && *data != '?' && *data != 0)
    {
        ++data;
    }
    if (*data == 0)
    {
        error = "Unexpected end of file";
        return false;
    }
    if (data[0] == '/' && data[1] != '>')
    {
        error = "Missing >";
        return false;
    }
    name = std::string(nameBegin, data - nameBegin);
    return true;
}

bool XmlDocument::parseAttributes(const char *& data, std::map<std::string, std::string> & attr)
{
    while(*data != '>' && *data != '/')
    {
        skipWhitespaces(data);
        std::string attrName;
        if(!parseName(data, attrName))
        {
            return false;
        }
        skipWhitespaces(data);

        if(*data != '=')
        {
            error = "Missing '=' after attribute name";
            return false;
        }
        ++data;

        skipWhitespaces(data);

        char quote = *data;
        if(quote != '\'' && quote != '"')
        {
            error = "Missing ' or \" before attribute value";
            return false;
        }
        ++data;
        const char * attrValueBegin = data;
        while(*data != quote && *data != 0)
        {
            ++data;
        }

        if(*data != quote)
        {
            error = "Missing second quote";
            return false;
        }

        attr[attrName] = std::string(attrValueBegin, data - attrValueBegin);
        ++data;

        skipWhitespaces(data);
    }
    return true;
}
