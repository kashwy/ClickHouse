#include <Parsers/Kusto/KQLTokenIterator.h>


namespace DB
{

KQLTokens::KQLTokens(const char * begin, const char * end, size_t max_query_size, bool skip_insignificant)
{
    KQLLexer lexer(begin, end, max_query_size);

    bool stop = false;
    do
    {
        KQLToken token = lexer.nextToken();
        stop = token.isEnd() || token.type == KQLTokenType::ErrorMaxQuerySizeExceeded;
        if (token.isSignificant() || (!skip_insignificant && !data.empty() && data.back().isSignificant()))
            data.emplace_back(std::move(token));
    } while (!stop);
}

KQLUnmatchedParentheses checkKQLUnmatchedParentheses(KQLTokenIterator begin)
{
    /// We have just two kind of parentheses: () and [].
    KQLUnmatchedParentheses stack;

    /// We have to iterate through all tokens until the end to avoid false positive "Unmatched parentheses" error
    /// when parser failed in the middle of the query.
    for (KQLTokenIterator it = begin; it.isValid(); ++it)
    {
        if (it->type == KQLTokenType::OpeningRoundBracket || it->type == KQLTokenType::OpeningSquareBracket)
        {
            stack.push_back(*it);
        }
        else if (it->type == KQLTokenType::ClosingRoundBracket || it->type == KQLTokenType::ClosingSquareBracket)
        {
            if (stack.empty())
            {
                /// Excessive closing bracket.
                stack.push_back(*it);
                return stack;
            }
            else if ((stack.back().type == KQLTokenType::OpeningRoundBracket && it->type == KQLTokenType::ClosingRoundBracket)
                || (stack.back().type == KQLTokenType::OpeningSquareBracket && it->type == KQLTokenType::ClosingSquareBracket))
            {
                /// Valid match.
                stack.pop_back();
            }
            else
            {
                /// Closing bracket type doesn't match opening bracket type.
                stack.push_back(*it);
                return stack;
            }
        }
    }

    /// If stack is not empty, we have unclosed brackets.
    return stack;
}

}
