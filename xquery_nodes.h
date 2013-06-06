#pragma once

#include <unordered_map>
#include <cassert>
#include <algorithm>

#include "xquery_ast.h"
#include "xquery_ast_utils.h"

namespace xquery { namespace lang
{

enum NTLabel // Non terminal labels
{
    AP,   // Absolute path
    RP,   // Relative path
    F,    // Filter
    XQ,   // XQuery
    COND  // Condition
};

class NonTerminalNode : public Node
{
    public:
        NonTerminalNode(NTLabel label, Edges&& edges)
          : Node{std::move(edges)},
            label_{label}
        {
            set_label("NonTerminalNode `" + kMap_.at(label_) + "'");
            assert(edges_.size() == 1);
        }
        ~NonTerminalNode() = default;

        EvalResult Eval(const EvalResult& res) const override;

    private:
        const std::unordered_map<NTLabel, std::string, std::hash<int>> kMap_= {
            {AP, "AP"},
            {RP, "RP"},
            {F, "F"},
            {XQ, "XQ"},
            {COND, "COND"}
        };
        NTLabel label_;
};

class TagName : public Node
{
    public:
        TagName(const std::string& tagname) : tagname_{tagname}
        {
            set_label("TagName `" + tagname_ + "'");
        }
        ~TagName() = default;

        EvalResult Eval(const EvalResult& res) const override;

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

        EvalResult Eval(const EvalResult& res) const override;
};

class Document : public Node
{
    public:
        Document(const std::string& name) : name_{name}
        {
            set_label("Document `" + name_ + "'");
        }
        ~Document() = default;

        EvalResult Eval(const EvalResult& res) const override;

    private:
        mutable xml::DomParser parser_;
        std::string            name_;
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
          : Node{std::move(edges)}
        {
            sep_ = kMap_.at(token);
            set_label("PathSeparator `" + token + "'");
            assert(edges_.size() == 2);
        }
        ~PathSeparator() = default;

        EvalResult Eval(const EvalResult& res) const override;

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

        EvalResult Eval(const EvalResult& res) const override;

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
        Precedence(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("Precedence");
            assert(edges_.size() == 1);
        }
        ~Precedence() = default;

        EvalResult Eval(const EvalResult& res) const override;
};

class Concatenation : public Node
{
    public:
        Concatenation(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("Concatenation");
            assert(edges_.size() == 2);
        }
        ~Concatenation() = default;

        EvalResult Eval(const EvalResult& res) const override;
};

class Filter : public Node
{
    public:
        Filter(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("Filter");
            assert(edges_.size() == 2);
        }
        ~Filter() = default;

        EvalResult Eval(const EvalResult& res) const override;
};

class Equality : public Node
{
    enum EqType
    {
        REF,
        VALUE
    };

    public:
        Equality(const std::string& token, Edges&& edges) : Node{std::move(edges)}
        {
            eq_ = kMap_.at(token);
            set_label("Equality `" + token + "'");
            assert(edges_.size() == 2);
        }
        ~Equality() = default;

        EvalResult Eval(const EvalResult& res) const override;

    private:
        bool HasValueEquality(const xml::Node* n1, const xml::Node* n2) const;

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
        LogicOperator(const std::string& token, Edges&& edges) : Node{std::move(edges)}
        {
            op_ = kMap_.at(token);
            set_label("LogicOperator `" + token + "'");
            assert(edges_.size() == 1 || edges_.size() == 2);
        }
        ~LogicOperator() = default;

        EvalResult Eval(const EvalResult& res) const override;

    private:
        const std::unordered_map<std::string, OpType> kMap_= {
            {"and", AND},
            {"or", OR},
            {"not", NOT}
        };
        OpType op_;
};

class Variable : public Node
{
    public:
        Variable(const std::string& varname) : varname_{varname}
        {
            set_label("Variable `" + varname_ + "'");
        }
        ~Variable() = default;

        EvalResult Eval(const EvalResult& res) const override;

    private:
        std::string varname_;
};

class ConstantString : public Node
{
    public:
        ConstantString(const std::string& cstring) : cstring_{cstring}
        {
            set_label("ConstantString `" + cstring_ + "'");
        }
        ~ConstantString() = default;

        EvalResult Eval(const EvalResult& res) const override;

    private:
        std::string cstring_;
};

class Tag : public Node
{
    public:
        Tag(const std::string& otagname, const std::string& ctagname, Edges&& edges)
            : Node{std::move(edges)},
              tagname_{otagname}
        {
            set_label("Tag `" + tagname_ + "'");
            assert(ctagname == otagname);
        }
        ~Tag() = default;

        EvalResult Eval(const EvalResult& res) const override;

    private:
        std::string tagname_;
};

class LetClause : public Node
{
    public:
        LetClause(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("LetClause");
            assert(edges_.size() >= 1);
        }
        ~LetClause() = default;

        EvalResult Eval(const EvalResult& res) const override;
};

class WhereClause : public Node
{
    public:
        WhereClause(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("WhereClause");
            assert(edges_.size() == 1);
        }
        ~WhereClause() = default;

        EvalResult Eval(const EvalResult& res) const override;
};

class ForClause : public Node, public ContextIterator
{
    public:
        ForClause(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("ForClause");
            assert(edges_.size() >= 1);
        }
        ~ForClause() = default;

        ctx_iterator ctx_begin() const
        {
            return ContextIterator::begin(this);
        }
        ctx_iterator ctx_end() const
        {
            return ContextIterator::end();
        }
        EvalResult Eval(const EvalResult& res) const override;
};

class ReturnClause : public Node
{
    public:
        ReturnClause(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("ReturnClause");
            assert(edges_.size() == 1);
        }
        ~ReturnClause() = default;

        EvalResult Eval(const EvalResult& res) const override;
};

class FLWRExpression : public Node
{
    public:
        FLWRExpression(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("FLWRExpression");
            assert(edges_.size() >= 2);
        }
        ~FLWRExpression() = default;

        EvalResult Eval(const EvalResult& res) const override;
};

class LetExpression : public Node
{
    public:
        LetExpression(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("LetExpression");
            assert(edges_.size() == 2);
        }
        ~LetExpression() = default;

        EvalResult Eval(const EvalResult& res) const override;
};

class VariableDef : public Node
{
    public:
        VariableDef(const std::string& varname, Edges&& edges)
          : Node{std::move(edges)},
            varname_{varname}
        {
            set_label("VariableDef");
            assert(edges_.size() == 1);
        }
        ~VariableDef() = default;

        EvalResult Eval(const EvalResult& res) const override;

    private:
        std::string varname_;
};

class SomeExpression : public Node
{
    public:
        SomeExpression(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("SomeExpression");
            assert(edges_.size() == 2);
        }
        ~SomeExpression() = default;

        EvalResult Eval(const EvalResult& res) const override;
};

class SomeClause : public Node, public ContextIterator
{
    public:
        SomeClause(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("SomeClause");
            assert(edges_.size() >= 1); 
        }
        ~SomeClause() = default;

        EvalResult Eval(const EvalResult& res) const override;

        ctx_iterator ctx_begin() const
        {
            return ContextIterator::begin(this);
        }
        ctx_iterator ctx_end() const
        {
            return ContextIterator::end();
        }
};

class Empty : public Node
{
    public:
        Empty(Edges&& edges) : Node{std::move(edges)}
        {
            set_label("Empty");
            assert(edges_.size() == 1);
        }
        ~Empty() = default;

        EvalResult Eval(const EvalResult& res) const override;
};

}}
