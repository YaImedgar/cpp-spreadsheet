#include "formula.h"
using namespace std::literals;

std::ostream &operator<<(std::ostream &output, FormulaError fe)
{
    return output << "#DIV/0!";
}

namespace
{
    class Formula : public FormulaInterface
    {
    public:
        explicit Formula(std::string expression)
            : ast_(ParseFormulaAST(expression))
        {}

        Value Evaluate(const SheetInterface& sheet) const override
        {
            try
            {
                return ast_.Execute(sheet);
            }
            catch (const FormulaError &fe)
            {
                return fe;
            }
        }

        std::string GetExpression() const override
        {
            std::stringstream ss;

            ss.clear();
            ast_.PrintFormula(ss);

            return ss.str();
        }

        std::vector<Position> GetReferencedCells() const override
        {
            auto unique = std::set(ast_.GetCells().begin(), ast_.GetCells().end());
            return {unique.begin(), unique.end()};
        }
    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression)
{
    return std::make_unique<Formula>(std::move(expression));
}
