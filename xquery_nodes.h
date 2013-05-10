#pragma once

#include <unordered_map>
#include <algorithm>

#include "xquery_ast.h"

namespace xquery { namespace lang
{

enum NTLabel // Non terminal labels
{
    AP, // Absolute path
    RP  // Relative path
};

class NonTerminalNode : public Node
{
    public:
        NonTerminalNode(NTLabel label, std::vector<const Node*>&& edges)
          : Node(std::forward<std::vector<const Node*>>(edges)),
            label_(label)
        {
            set_label("NonTerminalNode `" + kMap_.at(label) + "'");
        }
        ~NonTerminalNode() = default;

    private:
        const std::unordered_map<NTLabel, std::string, std::hash<int>> kMap_= {
            {AP, "AP"},
            {RP, "RP"}
        };
        NTLabel label_;
};

class TagName : public Node
{
    public:
        TagName(const std::string& tagname) : tagname_(tagname)
        {
            set_label("TagName `" + tagname_ +"'");
        }
        ~TagName() = default;

    private:
        std::string tagname_;
};

class Text : public Node
{
    public:
        Text()
        {
            set_label("Text");
        }
        ~Text() = default;
};

class Document : public Node
{
    public:
        Document(const std::string& name) : name_(name)
        {
            set_label("Document `" + name_ + "'");
        }
        ~Document() = default;

    private:
        std::string name_;
};

class PathSeparator : public Node
{
    enum SepType
    {
        DESC,
        DESC_OR_SELF
    };

    public:
        PathSeparator(const std::string& token)
        {
            sep_ = kMap_.at(token);
            set_label("PathSeparator `" + token + "'");
        }
        ~PathSeparator() = default;

    private:
        const std::unordered_map<std::string, SepType> kMap_= {
            {"/", DESC},
            {"//", DESC_OR_SELF}
        };
        SepType sep_;
};

class PathGlobbing : public Node
{
    enum GlobType
    {
        PARENT,
        SELF,
        WILDCARD
    };

    public:
        PathGlobbing(const std::string& token)
        {
            glob_ = kMap_.at(token);
            set_label("PathGlobbing `" + token + "'");
        }
        ~PathGlobbing() = default;

    private:
        const std::unordered_map<std::string, GlobType> kMap_= {
            {"*", WILDCARD},
            {".", SELF},
            {"..", PARENT}
        };
        GlobType glob_;
};

}}
