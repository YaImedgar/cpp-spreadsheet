#include "sheet.h"

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Bad position value"s);
    }

    if (!sheet_.count(pos))
    {
        auto cell = std::make_unique<Cell>(*this);
        cell->Set(std::move(text), pos);
        sheet_.emplace(pos, std::move(cell));
        sheet_map_[pos] = sheet_.at(pos).get();
        row_count_[pos.row]++;
        col_count_[pos.col]++;
        cleared_cells_.erase(pos);
        return;
    }

    auto &cell = sheet_[pos];
    cell->Set(std::move(text), pos);
}

const CellInterface *Sheet::GetCell(Position pos) const
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Bad position value"s);
    }

    if (!sheet_.count(pos))
    {
        return nullptr;
    }

    return sheet_.at(pos).get();
}

CellInterface *Sheet::GetCell(Position pos)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Bad position value"s);
    }

    if (cleared_cells_.count(pos) || !sheet_.count(pos))
    {
        return nullptr;
    }

    return sheet_.at(pos).get();
}

void Sheet::ClearCell(Position pos)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Bad position value"s);
    }

    if (!sheet_.count(pos))
    {
        return;
    }

    cleared_cells_.emplace(pos);
    sheet_.erase(pos);
    sheet_map_.erase(pos);
    RemoveOneFrom(row_count_, pos.row);
    RemoveOneFrom(col_count_, pos.col);
}

Size Sheet::GetPrintableSize() const
{
    if (sheet_.empty())
    {
        return {};
    }

    return {row_count_.rbegin()->first + 1, col_count_.rbegin()->first + 1};
}

void Sheet::PrintValues(std::ostream &output) const
{
    if (sheet_.empty())
    {
        return;
    }

    PrintTo(output, TypeOfCell::VALUE);
}

void Sheet::PrintTexts(std::ostream &output) const
{
    if (sheet_.empty())
    {
        return;
    }

    PrintTo(output, TypeOfCell::TEXT);
}

void Sheet::RemoveOneFrom(std::map<int, int> &map, int position)
{
    auto iter = map.find(position);
    auto &pos = iter->second;
    --pos;
    if (pos == 0)
    {
        map.erase(iter);
    }
}

void Sheet::PrintRowTo(std::ostream &ostream, int row, TypeOfCell type) const
{
    int last_col = col_count_.rbegin()->first;
    for (int col = 0; col <= last_col; ++col)
    {
        Position find_pos = {row, col};
        if (!sheet_.count(find_pos))
        {
            if (col != last_col)
            {
                ostream << "\t";
            }
            continue;
        }

        const auto &cell = sheet_.at(find_pos);
        switch (type)
        {
            case TypeOfCell::NONE:
                [[fallthrough]];
            default:
                break;
            case TypeOfCell::TEXT:
                ostream << cell->GetText();
                break;
            case TypeOfCell::VALUE:
                auto value = cell->GetValue();
                if (std::holds_alternative<double>(value))
                {
                    ostream << std::get<double>(value);
                } else if (std::holds_alternative<std::string>(value))
                {
                    ostream << std::get<std::string>(value);
                } else if (std::holds_alternative<FormulaError>(value))
                {
                    ostream << std::get<FormulaError>(value);
                }
                break;
        }
        if (col != last_col)
        {
            ostream << "\t";
        }
    }
    ostream << "\n";
}

void Sheet::PrintTo(std::ostream &output, TypeOfCell type) const
{
    int last_row = row_count_.rbegin()->first;
    for (int row = 0; row <= last_row; ++row)
    {
        PrintRowTo(output, row, type);
    }
}

double Sheet::operator()(Position pos) const
{
    if (!pos.IsValid())
    {
        throw FormulaError(FormulaError::Category::Ref);
    }

    if (!sheet_.count(pos))
    {
        return 0;
    }
    const CellInterface *cell = GetCell(pos);
    CellInterface::Value value = cell->GetValue();

    if (const auto double_val = std::get_if<double>(&value))
    {
        return *double_val;
    }
    else if (const auto it = std::get_if<std::string>(&value))
    {
        if (!std::all_of(it->begin(), it->end(), ::isdigit))
        {
            throw FormulaError(FormulaError::Category::Value);
        }

        if (*it == "")
        {
            return 0;
        }

        double res = 0;
        try
        {
            res = std::stod(*it);
        }
        catch (...)
        {
            throw FormulaError(FormulaError::Category::Value);
        }
        return res;
    }
    else
    {
        throw std::get<FormulaError>(value);
    }
}

std::unique_ptr<SheetInterface> CreateSheet()
{
    return std::make_unique<Sheet>();
}