#include "stdafx.h"


struct parser
{
    typedef vector<string>::const_iterator it_t;
    virtual bool parse(it_t begin, it_t end, it_t &out_it) = 0;
    virtual ~parser() {};
};

typedef std::shared_ptr<parser> parser_ptr;

struct parser_word
    : parser
{
    parser_word(string const &word)
        : word_(word)
    {}

    bool parse(it_t begin, it_t end, it_t &out_it) override
    {
        out_it = begin;
        
        if (begin == end)
            return false;

        if (*out_it == word_)
        {
            ++out_it;
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    string word_;
};

struct parser_cat
    : parser
{
    parser_cat(parser_ptr p1, parser_ptr p2)
        : p1_(p1)
        , p2_(p2)
    {}

    bool parse(it_t begin, it_t end, it_t &out_it) override
    {
        out_it = begin;
        if (!p1_->parse(out_it, end, out_it))
            return false;
        if (!p2_->parse(out_it, end, out_it))
            return false;
        return true;
    }

private:
    parser_ptr p1_, p2_;
};

struct parser_or
    : parser
{
    parser_or(parser_ptr p1, parser_ptr p2)
        : p1_(p1)
        , p2_(p2)
    {}

    bool parse(it_t begin, it_t end, it_t &out_it) override
    {
        out_it = begin;
        if (p1_->parse(out_it, end, out_it))
            return true;

        out_it = begin;
        return p2_->parse(out_it, end, out_it);
    }

private:
    parser_ptr p1_, p2_;
};

struct parser_opt
    : parser
{
    parser_opt(parser_ptr p)
        : p_(p)
    {}

    bool parse(it_t begin, it_t end, it_t &out_it) override
    {
        out_it = begin;
        p_->parse(begin, end, out_it);
        return true;                
    }

private:
    parser_ptr p_;
};

struct parser_rep
    : parser
{
    parser_rep(parser_ptr p)
        : p_(p)
    {}

    bool parse(it_t begin, it_t end, it_t &out_it) override
    {
        out_it = begin;
        while(p_->parse(out_it, end, out_it)) 
        {

        }
        return true;                
    }

private:
    parser_ptr p_;
};

struct rule_t
{
    typedef parser::it_t it_t;

    rule_t(parser_ptr parser)
        : parser_(parser)
    {}

    bool parse(it_t begin, it_t end, it_t &out_it) const
    {
        return parser_->parse(begin, end, out_it);
    }
   
    rule_t operator+(rule_t const &r2) const
    {
        return rule_t(make_shared<parser_cat>(parser_, r2.parser_));
    }
    
    rule_t operator|(rule_t const &r2) const
    {
        return rule_t(make_shared<parser_or>(parser_, r2.parser_));
    }

    rule_t operator!() const
    {
        return rule_t(make_shared<parser_opt>(parser_));
    }

    rule_t operator*() const
    {
        return rule_t(make_shared<parser_rep>(parser_));
    }

private:
    parser_ptr parser_;
};

template<typename T, typename ...Args>
rule_t mp(Args... args)
{
    return rule_t(make_shared<T>(args...));
}


int main() 
{
    vector<string> strings = {"hello", "hello", "hello", "world", "how", /*"the", "fuck",*/ "are", "you"};
    auto rule = mp<parser_word>("hello")
              + *mp<parser_word>("hello")
              + !mp<parser_word>("brave")
              + !mp<parser_word>("new")
              + mp<parser_word>("world")
              + (mp<parser_word>("how") | mp<parser_word>("where"))
              + !(mp<parser_word>("the") + mp<parser_word>("fuck"))
              + mp<parser_word>("are")
              + mp<parser_word>("you");
    
    vector<string>::const_iterator it;
    if (rule.parse(strings.begin(), strings.end(), it) && it == strings.end())
    {
        cout << "success!" << endl;
    }
}
