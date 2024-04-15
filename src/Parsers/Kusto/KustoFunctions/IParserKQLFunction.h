#pragma once

#include <Parsers/Kusto/IKQLParserBase.h>

#include <span>

namespace DB
{
class Interval
{
public:
    using Representation = int;

    Interval(const Representation min_, const Representation max_) : max(max_), min(min_) { }

    Representation Max() const { return max; }
    Representation Min() const { return min; }
    bool IsWithinBounds(const Representation value) const { return min <= value && value <= max; }

    static constexpr auto max_bound = std::numeric_limits<Representation>::max();
    static constexpr auto min_bound = std::numeric_limits<Representation>::min();

private:
    Representation max = max_bound;
    Representation min = min_bound;
};

class IParserKQLFunction
{
public:
    enum class ArgumentState
    {
        Parsed,
        Raw
    };

    template <typename F>
    ALWAYS_INLINE static bool wrapConvertImpl(IKQLParser::KQLPos & pos, const F & func)
    {
        IKQLParser::KQLPos begin = pos;
        bool res = func();
        if (!res)
            pos = begin;
        return res;
    }

    struct IncreaseDepthTag
    {
    };

    template <typename F>
    ALWAYS_INLINE static bool wrapConvertImpl(IKQLParser::KQLPos & pos, IncreaseDepthTag, const F & func)
    {
        IKQLParser::KQLPos begin = pos;
        pos.increaseDepth();
        bool res = func();
        pos.decreaseDepth();
        if (!res)
            pos = begin;
        return res;
    }

    bool convert(String & out, IKQLParser::KQLPos & pos);
    virtual const char * getName() const = 0;
    virtual ~IParserKQLFunction() = default;

    static String generateUniqueIdentifier();
    static String getArgument(const String & function_name, DB::IKQLParser::KQLPos & pos, ArgumentState argument_state = ArgumentState::Parsed);
    static std::vector<std::string> getArguments(
        const String & function_name,
        DB::IKQLParser::KQLPos & pos,
        ArgumentState argument_state = ArgumentState::Parsed,
        const Interval & argument_count_interval = {0, Interval::max_bound});
    static String getConvertedArgument(const String & fn_name, IKQLParser::KQLPos & pos);
    static String getExpression(IKQLParser::KQLPos & pos);
    static String getKQLFunctionName(IKQLParser::KQLPos & pos);
    static std::optional<String>
    getOptionalArgument(const String & function_name, DB::IKQLParser::KQLPos & pos, ArgumentState argument_state = ArgumentState::Parsed);
    static String
    kqlCallToExpression(std::string_view function_name, std::initializer_list<const std::string_view> params, uint32_t max_depth, uint32_t max_backtracks);
    static String kqlCallToExpression(std::string_view function_name, std::span<const std::string_view> params, uint32_t max_depth, uint32_t max_backtracks);
    static String escapeSingleQuotes(const String & input);

protected:
    virtual bool convertImpl(String & out, IKQLParser::KQLPos & pos) = 0;

    static bool directMapping(
        String & out, IKQLParser::KQLPos & pos, std::string_view ch_fn, const Interval & argument_count_interval = {0, Interval::max_bound});
    static void validateEndOfFunction(const String & fn_name, IKQLParser::KQLPos & pos);
};
}
