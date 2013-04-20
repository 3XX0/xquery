#pragma once

#include <unordered_map>
#include <algorithm>

#include "xquery_ast.h"

#define __CLASS__ (typeid(*this).name())

#define REVERSE_AT(x, y) \
    std::find_if(std::begin(x), std::end(x), \
    [this] (decltype(*std::begin(x)) n) { \
    return (n.second == y); })->first;

namespace XQuery { namespace Lang
{

enum class TLabel
{
    AP,
    RP
};

class NonTerminalNode : public Node
{
    public:
        NonTerminalNode(TLabel label, std::vector<const Node*>&& edges)
          : Node(std::forward<std::vector<const Node*>>(edges)),
            label_(label) {}
        ~NonTerminalNode() = default;

        virtual void trace() const override
        {
            std::cout << __CLASS__ << " [" <<
              (label_ == TLabel::AP ? "AP" : "RP") << "]" << std::endl;
        }

    private:
        TLabel label_;
};

class TagName : public Node
{
    public:
        TagName(const std::string& tagname) : tagname_(tagname) {}
        ~TagName() = default;

        virtual void trace() const override
        {
            std::cout << __CLASS__ << " [" << tagname_ << "]" << std::endl;
        }

    private:
        std::string tagname_;
};

class Text : public Node
{
    public:
        Text() = default;
        ~Text() = default;

        virtual void trace() const override
        {
            std::cout << __CLASS__ << std::endl;
        }
};

class Document : public Node
{
    public:
        Document(const std::string& name) : name_(name) {}
        ~Document() = default;

        virtual void trace() const override
        {
            std::cout << __CLASS__ << " [" << name_ << "]" << std::endl;
        }

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
            sep_ = map_.at(token);
        }
        ~PathSeparator() = default;

        virtual void trace() const override
        {
            const std::string& sep_str = REVERSE_AT(map_, sep_);
            std::cout << __CLASS__ << " [" << sep_str << "]" << std::endl;
        }

    private:
        const std::unordered_map<std::string, SepType> map_= {
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
            glob_ = map_.at(token);
        }
        ~PathGlobbing() = default;

        virtual void trace() const override
        {
            const std::string& glob_str = REVERSE_AT(map_, glob_);
            std::cout << __CLASS__ << " [" << glob_str << "]" << std::endl;
        }

    private:
        const std::unordered_map<std::string, GlobType> map_= {
            {"*", WILDCARD},
            {".", SELF},
            {"..", PARENT}
        };
        GlobType glob_;
};

}}
