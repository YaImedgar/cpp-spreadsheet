#include "cell.h"

class Cell::Impl
{
public:
    virtual ~Impl() = default;

    virtual Value GetValue() const = 0;

    virtual std::string GetText() const = 0;

    virtual std::vector <Position> GetReferencedCells() const
    {
        return {};
    }
};

class Cell::EmptyImpl final : public Impl
{
public:
    Value GetValue() const override
    {
        return std::string{};
    }

    std::string GetText() const override
    {
        return std::string{};
    }
};

class Cell::TextImpl final : public Impl
{
public:
    TextImpl(std::string expression)
        : text_(std::move(expression))
    {}

    Value GetValue() const override
    {
        if (text_[0] == ESCAPE_SIGN)
        {
            return text_.substr(1);
        }

        return text_;
    }

    std::string GetText() const override
    {
        return text_;
    }

private:
    std::string text_;
};

class Cell::FormulaImpl final : public Impl
{
public:
    FormulaImpl(const Sheet &sheet, std::string expression)
        : sheet_(sheet)
    {
        formula_ = ParseFormula(expression.substr(1));
        referenced_cells_ = formula_->GetReferencedCells();
    }

    Value GetValue() const override
    {
        auto raw_value = formula_->Evaluate((const SheetInterface &) sheet_);
        if (std::holds_alternative<double>(raw_value))
        {
            return std::get<double>(raw_value);
        }
        return std::get<FormulaError>(raw_value);
    }

    std::string GetText() const override
    {
        return FORMULA_SIGN + formula_->GetExpression();
    }

    std::vector <Position> GetReferencedCells() const override
    {
        return referenced_cells_;
    }

private:
    const Sheet &sheet_;
    std::unique_ptr <FormulaInterface> formula_;
    std::vector <Position> referenced_cells_;
};

Cell::Cell(Sheet &sheet)
    : sheet_(sheet), impl_(std::make_unique<EmptyImpl>())
{}

Cell::~Cell()
{}

void Cell::Set(std::string text, Position pos)
{
    if (text == GetText())
    {
        return;
    }

    std::unique_ptr<Impl> temp_impl = std::make_unique<EmptyImpl>();

    if (text.size() > 1 && text[0] == FORMULA_SIGN)
    {
        temp_impl = std::make_unique<FormulaImpl>(sheet_, text);
        CheckCircularDependencies(temp_impl->GetReferencedCells(), this, pos);
    } else if (!text.empty())
    {
        temp_impl = std::make_unique<TextImpl>(text);
    }

    impl_ = std::move(temp_impl);

    referenced_cells_ = impl_->GetReferencedCells();
    InvalidateCashe();
}

void Cell::Clear()
{
    impl_.release();
}

Cell::Value Cell::GetValue() const
{
    return impl_->GetValue();
}

std::string Cell::GetText() const
{
    return impl_->GetText();
}

std::vector <Position> Cell::GetReferencedCells() const
{
    return referenced_cells_;
}

bool Cell::IsReferenced() const
{
    return !referenced_cells_.empty();
}

void Cell::InvalidateCashe()
{

}

void Cell::CheckCircularDependencies(std::vector <Position> cells, const CellInterface *head, Position pos)
{
    for (const Position &cell_pos: cells)
    {
        auto find_cell = sheet_.GetCell(cell_pos);

        if (find_cell == head || cell_pos == pos)
        {
            throw CircularDependencyException("Error");
        }

        if (find_cell == nullptr)
        {
            continue;
        }

        CheckCircularDependencies(find_cell->GetReferencedCells(), head, pos);
    }
}
