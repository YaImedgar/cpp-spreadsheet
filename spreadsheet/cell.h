#pragma once

#include "common.h"
#include "formula.h"

#include <set>
#include <string>
#include <optional>

class Cell : public CellInterface
{
public:
    Cell(Sheet &sheet);
    ~Cell();
    void Set(std::string text, Position pos);
    void Clear();
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    bool IsReferenced() const;
private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    Sheet &sheet_;
    std::unique_ptr<Impl> impl_;
    std::vector<Position> referenced_cells_{};

    void CheckCircularDependencies(std::vector<Position> cells, const CellInterface* head, Position pos);
    void InvalidateCashe();
};

#include "sheet.h"