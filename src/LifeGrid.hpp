/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2026 RPf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <cairommconfig.h>
#include <cairomm/surface.h>
#include <glibmm.h>
#include <memory>
#include <vector>
#include <unordered_set>




class LifeRule
{
public:
    LifeRule() = default;
    explicit LifeRule(const LifeRule& orig) = delete;
    virtual ~LifeRule() = default;
    virtual bool isAlive(bool life, int32_t neighbours) = 0;
    virtual std::string getName() = 0;
};

// conways basic rule
class LifeRuleB3_S23
: public LifeRule
{
public:
    LifeRuleB3_S23() = default;
    explicit LifeRuleB3_S23(const LifeRuleB3_S23& orig) = delete;
    virtual ~LifeRuleB3_S23() = default;
    std::string getName() override;
    bool isAlive(bool life, int32_t neighbours) override;
};

class DynamicLifeRule
    : public LifeRule
{
public:
    DynamicLifeRule(const std::string& rule);
    explicit DynamicLifeRule(const DynamicLifeRule& orig) = delete;
    virtual ~DynamicLifeRule() = default;
    std::string getMessage();
    std::string getName() override;
    bool isAlive(bool life, int32_t neighbours) override;
protected:
    void add(std::vector<bool>& set, const std::string& num);

private:
    std::vector<bool> m_birth;  // this seems to be the fastest lookup
    std::vector<bool> m_survival;
    std::string m_name;
    std::string m_msg;
};


class LifeGrid
{
public:
    LifeGrid(int32_t width, int32_t height);
    explicit LifeGrid(const LifeGrid& orig) = delete;
    virtual ~LifeGrid() = default;

    int32_t getHeight() const;
    int32_t getWidth() const;
    int32_t getGeneration() const;
    double getComputeTime() const;
    void fill(bool state = false);
    void fillRandom(int32_t randomness);
    bool nextGen();
    void update(Cairo::RefPtr<Cairo::ImageSurface>& m_imageSurface, bool renderWithColor);
    void setCell(int32_t col, int32_t row, int32_t n, bool value);

    std::shared_ptr<LifeRule> getRule() const;
    void setRule(const std::shared_ptr<LifeRule>& rule);
    std::vector<bool> getGrid() const
    {
        return m_grid;
    }
    std::vector<bool>& getGridRef()
    {
        return m_grid;
    }
    std::vector<bool>& getChangedRef()
    {
        return m_changed;
    }
    inline size_t getOffs(int32_t row, int32_t col) {
        return static_cast<size_t>(row * m_width + col);
    }
protected:
    std::unique_ptr<std::vector<int32_t>> getRowCount(int32_t row);
    size_t getAllocation() const;

    private:
    int32_t m_width;
    int32_t m_height;
    int32_t m_generation{};
    double m_computeTime{};
    // the storage options are
    // --- enum based on uint8_t
    //  # enum with default, inefficent storage, but fast computation
    //  # separate bool's compact storage, time ~ as above
    std::vector<bool> m_grid;
    std::vector<bool> m_changed;
    std::shared_ptr<LifeRule> m_rule;
};

