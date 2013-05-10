#pragma once

#include <unordered_map>
#include <algorithm>

#include "xquery_ast.h"

namespace xquery { namespace lang
{

enum NTLabel // Non terminal labels
{
    AP, // Absolute path
    RP, // Relative path
    F   // Filter
};

typedef std::vector<const Node*> Edges;

class NonTerminalNode : public Node
{
    public:
        NonTerminalNode(NTLabel label, Edges&& edges)
          : Node(std::forward<Edges>(edges)),
            label_(label)
        {
            set_label("NonTerminalNode `" + kMap_.at(label) + "'"); 
        }
        ~NonTerminalNode() = default;

    private:
        const std::unordered_map<NTLabel, std::string, std::hash<int>> kMap_= {
            {AP, "AP"},
            {RP, "RP"},
            {F, "F"}
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
        PathSeparator(const std::string& token, Edges&& edges)
          : Node(std::forward<Edges>(edges))
        {
            sep_ = kMap_.at(token);
            set_label("PathSeparator `" + token + "'");
            assert(edges_.size() == 2);
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

class Precedence : public Node
{
    public:
        Precedence(Edges&& edges) : Node(std::forward<Edges>(edges))
        {
            set_label("Precedence");
            assert(edges_.size() == 1);
        }
        ~Precedence() = default;
};

class Concatenation : public Node
{
    public:
        Concatenation(Edges&& edges) : Node(std::forward<Edges>(edges))
        {
            set_label("Concatenation");
            assert(edges_.size() == 2);
        }
        ~Concatenation() = default;
};

class Filter : public Node
{
    public:
        Filter(Edges&& edges) : Node(std::forward<Edges>(edges))
        {
            set_label("Filter");
            assert(edges_.size() == 2);
        }
        ~Filter() = default;
};

class Equality : public Node
{
    enum EqType
    {
        REF,
        VALUE
    };

    public:
        Equality(const std::string& token, Edges&& edges) : Node(std::forward<Edges>(edges))
        {
            eq_ = kMap_.at(token);
            set_label("Equality `" + token + "'");
            assert(edges_.size() == 2);
        }
        ~Equality() = default;

    private:
        const std::unordered_map<std::string, EqType> kMap_= {
            {"=", VALUE},
            {"eq", VALUE},
            {"==", REF},
            {"is", REF}
        };
        EqType eq_;
};

class LogicOperator : public Node
{
    enum OpType
    {
        AND,
        OR,
        NOT
    };

    public:
        LogicOperator(const std::string& token, Edges&& edges) : Node(std::forward<Edges>(edges))
        {
            op_ = kMap_.at(token);
            set_label("LogicOperator `" + token + "'");
            assert(edges_.size() == 1 || edges_.size() == 2);
        }
        ~LogicOperator() = default;

    private:
        const std::unordered_map<std::string, OpType> kMap_= {
            {"and", AND},
            {"or", OR},
            {"not", NOT}
        };
        OpType op_;
};

}}
