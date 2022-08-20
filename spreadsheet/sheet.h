#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>
#include <deque>
#include <map>
#include <iostream>
#include <optional>

class Sheet : public SheetInterface {
public:
    Sheet() = default;
    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    double operator()(Position pos) const override;
private:
    enum class TypeOfCell
    {
        NONE,
        TEXT,
        VALUE
    };

    void RemoveOneFrom(std::map<int, int> &map, int row);
    void PrintRowTo(std::ostream &ostream, int row, TypeOfCell type) const;
    void PrintTo(std::ostream &output, TypeOfCell type) const;

    std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> sheet_;
    std::unordered_set<Position, PositionHasher> cleared_cells_;
    std::map<Position, Cell const *> sheet_map_;
    // row, number of rows
    std::map<int, int> row_count_;
    // col, number of cols
    std::map<int, int> col_count_;
};