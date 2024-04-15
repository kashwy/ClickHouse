#pragma once

#include <Parsers/Kusto/IKQLParserBase.h>
#include <Parsers/Kusto/ParserKQLProject.h>
#include <Parsers/Kusto/ParserKQLQuery.h>

namespace DB
{

class ParserKQLExtend : public ParserKQLBase
{
protected:
    const char * getName() const override { return "KQL extend"; }
    bool parseImpl(KQLPos & pos, ASTPtr & node, KQLExpected & expected) override;
};

}
