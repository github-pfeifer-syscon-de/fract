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

#include <iostream>
#include <random>
#include <chrono>

#include "LifeGrid.hpp"


std::string
LifeRule23::getName()
{
    return "B3/S23";
}

bool
LifeRule23::isAlive(bool life, int32_t neighbours)
{
    bool nextLife;
    if (life) {
        nextLife = neighbours >= 2 && neighbours <= 3;
    }
    else {
        nextLife = neighbours == 3;
    }
    return nextLife;
}

DynamicLifeRule::DynamicLifeRule(const std::string& rule)
: m_birth(10, false)
, m_survival(10, false)
, m_name{rule}
{
    auto bpos = rule.find('b');
    auto slpos = rule.find('/');
    if (bpos != rule.npos) {    // try "b3/s23" without presuming specific order
        ++bpos;
        auto end = rule.find('/', bpos);
        if (end == rule.npos) {
            end = rule.size();
        }
        auto birth = rule.substr(bpos, end - bpos);
        try {
            add(m_birth, birth);
        }
        catch (std::invalid_argument& e) {
            m_msg += Glib::ustring::sprintf("Rule not parsable birth number %s\n", birth);
        }
    }
    auto spos = rule.find('s');
    if (spos != rule.npos) {    // try "b3/s23" without presuming specific order
        ++spos;
        auto end = rule.find('/', spos);
        if (end == rule.npos) {
            end = rule.size();
        }
        auto survival = rule.substr(spos, end - spos);
        try {
            add(m_survival, survival);
        }
        catch (std::invalid_argument& e) {
            m_msg += Glib::ustring::sprintf("Rule not parsable survival number %s\n", survival);
        }
    }
    else if (slpos != rule.npos) {   // try "23/3" as Survival/Birth, used by life 1.05,rle
        auto survival = rule.substr(0, slpos);
        try {
            add(m_survival, survival);
        }
        catch (std::invalid_argument& e) {
            m_msg += Glib::ustring::sprintf("Rule not parsable survival number %s\n", survival);
        }
        auto birth = rule.substr(slpos + 1);
        try {
            add(m_birth, birth);
        }
        catch (std::invalid_argument& e) {
            m_msg += Glib::ustring::sprintf("Rule not parsable birth number %s\n", birth);
        }

    }
    if (m_birth.empty() || m_survival.empty()) {
        m_msg += Glib::ustring::sprintf("Rule incomplete %s\n", rule);
    }
}

void
DynamicLifeRule::add(std::vector<bool>& set, const std::string& num)
{
    for (std::string::size_type i = 0; i < num.length(); ++i) {
        auto n = std::stoi(num.substr(i, 1));
        set[n] = true;
    }
}


std::string
DynamicLifeRule::getMessage()
{
    return m_msg;
}
std::string
DynamicLifeRule::getName()
{
    return m_name;
}

bool
DynamicLifeRule::isAlive(bool life, int32_t neighbours)
{
    bool nextLife;
    if (life) {
        nextLife = m_survival[neighbours];
    }
    else {
        nextLife = m_birth[neighbours];
    }
    return nextLife;
}


LifeGrid::LifeGrid(int32_t width, int32_t height)
: m_width{width}
, m_height{height}
, m_grid(width * height, false)
, m_changed(width * height, false)
, m_rule{std::make_shared<LifeRule23>()}
{
    allocate();
}

void
LifeGrid::allocate()
{
    m_scaleFactor = std::max(1, 512 / std::max(m_width, m_height));
}

size_t
LifeGrid::getAllocation() const
{
    return m_width * m_height;
}

int32_t
LifeGrid::getWidth() const
{
    return m_width;
}

int32_t
LifeGrid::getHeight() const
{
    return m_height;
}

int32_t
LifeGrid::getScaleFactor() const
{
    return m_scaleFactor;
}

int32_t
LifeGrid::getGeneration() const
{
    return m_generation;
}

double
LifeGrid::getComputeTime() const
{
    return  m_computeTime;
}

void
LifeGrid::fill(bool state)
{
    std::fill(m_grid.begin(), m_grid.end(), state);
    std::fill(m_changed.begin(), m_changed.end(), state);
    m_generation = 0;
}

void
LifeGrid::fillRandom(int32_t randomness)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    auto mid = randomness / 2u;
    for (uint32_t i = 0; i < m_grid.size(); ++i) {
        auto val = rng() % randomness;
        m_grid[i] = (val == mid);
        m_changed[i] = (val == mid);
    }
    m_generation = 0;
}

// remember sums avoid multiple index eval
//   the calculation of prepared sums reduces times from ~3ms to ~2ms
std::unique_ptr<std::vector<int32_t>>
LifeGrid::getRowCount(int32_t row)
{
    auto rowCnt = std::make_unique<std::vector<int32_t>>(m_width);
    const auto rowsOffs = getOffs(row, 0);
    int32_t prevCellCnt = m_grid[rowsOffs + (m_width - 1)] ? 1 : 0;
    int32_t thisCellCnt = m_grid[rowsOffs + 0] ? 1 : 0;
    for (int32_t cell = 0; cell < m_width; ++cell) {
        const auto nextCell = cell < m_width - 1 ? cell + 1 : 0;
        int32_t nextCellCnt = m_grid[rowsOffs + nextCell] ? 1 : 0;
        (*rowCnt)[cell] = prevCellCnt + thisCellCnt + nextCellCnt;
        prevCellCnt = thisCellCnt;
        thisCellCnt = nextCellCnt;
    }
    return rowCnt;
}

bool
LifeGrid::nextGen()
{
    const auto start{std::chrono::steady_clock::now()};
    bool anySet{};
    auto prevRowCnt = getRowCount(m_height - 1);   // wrap at edges
    auto zeroRowCnt = getRowCount(0);   // since we are overwriting cells need stored instance
    auto thisRowCnt = getRowCount(0);
    for (int32_t row = 0; row < m_height; ++row) {
        const auto rowsOffs = getOffs(row, 0);
        std::unique_ptr<std::vector<int32_t>> nextRowCnt;
        if (row < m_height - 1) {
            nextRowCnt = getRowCount(row + 1);
        }
        else {
            nextRowCnt = std::move(zeroRowCnt);
        }
        for (int32_t cell = 0; cell < m_width; ++cell) {
            auto cnt = (*prevRowCnt)[cell] + (*thisRowCnt)[cell] + (*nextRowCnt)[cell];
            auto alive = m_grid[rowsOffs + cell];
            if (alive) {
                --cnt;  // here we count the cell we are on as well
            }
            bool set = m_rule->isAlive(alive, cnt);
            if (alive != set) {
                m_grid[rowsOffs + cell] = set;
                m_changed[rowsOffs + cell] = true;
            }
            else {
                m_changed[rowsOffs + cell] = false;
            }
            anySet |= set;
        }
        prevRowCnt = std::move(thisRowCnt);
        thisRowCnt = std::move(nextRowCnt);
    }
    const auto end{std::chrono::steady_clock::now()};
    auto duration = std::chrono::duration<double>(end - start);
    //std::cout << "nextGen "  << duration.count() << "s" << std::endl;
    ++m_generation;
    m_computeTime = (m_computeTime * static_cast<double>(m_generation - 1) + duration.count()) / static_cast<double>(m_generation);

    return anySet;
}

void
LifeGrid::update(Cairo::RefPtr<Cairo::ImageSurface>& imageSurface, bool renderWithColor)
{
    const auto start{std::chrono::steady_clock::now()};
    //auto cr = Cairo::Context::create(imageSurface);
    // pushing pixels is 10*faster than calling cairo
    auto data = reinterpret_cast<uint32_t*>(imageSurface->get_data());
    uint32_t rgb;
    //double red;
    //double green;
    //double blue;
    const auto rowStride = (imageSurface->get_stride() / sizeof(int32_t));
    for (int32_t row = 0; row < m_height; ++row) {
        const auto cellOffs = getOffs(row, 0);
        for (int32_t cell = 0; cell < m_width; ++cell) {
            auto cellVal = m_grid[cellOffs + cell];
            auto cellChanged = m_changed[cellOffs + cell];
            if (renderWithColor) {
                //red = cellVal == LifeCell::On || cellVal == LifeCell::Vanished ? 1.0 : 0.0;
                //green = cellVal == LifeCell::On || cellVal == LifeCell::Generated ? 1.0 : 0.0;
                //blue = cellVal == LifeCell::On ? 1.0 : 0.0;
                rgb = 0xff000000
                    | (((cellVal && !cellChanged) || (!cellVal && cellChanged)) ? 0xff0000 : 0x0)
                    | (cellVal ? 0x00ff00 : 0x0)
                    | ((cellVal && !cellChanged) ? 0x0000ff : 0x0);
            }
            else {
                rgb =  0xff000000
                    | (cellVal ? 0xff0000 : 0x0)
                    | (cellVal ? 0x00ff00 : 0x0)
                    | (cellVal ? 0x0000ff : 0x0);
            }
            for (int32_t j = 0; j < m_scaleFactor; ++j) {
                auto rowPixOffs = data + ((row * m_scaleFactor + j) * rowStride) + cell * m_scaleFactor;
                for (int32_t i = 0; i < m_scaleFactor; ++i) {
                    rowPixOffs[i] = rgb;
                }
            }
            //cr->set_source_rgb(red, green, blue);
            //cr->rectangle(row * m_scaleFactor, cell * m_scaleFactor, m_scaleFactor, m_scaleFactor);
            //cr->fill();
        }
        imageSurface->mark_dirty(0, row, imageSurface->get_width(), m_scaleFactor);
    }
    const auto end{std::chrono::steady_clock::now()};
    auto duration = std::chrono::duration<double>(end - start);
    //std::cout << "update "  << duration.count() << "s" << std::endl;
}

void
LifeGrid::set(double eventX, double eventY, bool set)
{
    auto ix = static_cast<int32_t>(eventX / static_cast<double>(m_scaleFactor));
    auto iy = static_cast<int32_t>(eventY / static_cast<double>(m_scaleFactor));
    if (ix >= 0 && ix < static_cast<int32_t>(m_width)
     && iy >= 0 && iy < static_cast<int32_t>(m_height)) {
        const auto rowsOffs = getOffs(iy, ix);
        m_grid[rowsOffs] = set;
        m_changed[rowsOffs] = true;
        m_generation = 0;
     }
}

void
LifeGrid::setCell(int32_t col, int32_t row, int32_t n, bool value)
{
    auto i = getOffs(row, col);
    for ( ; n > 0; --n ) {
        if (i < m_grid.size()) {
            m_grid[i] = value;
            m_changed[i] = true;
        }
        else {
            std::cout <<  "LifeGrid::setCell index overrun " << i << " col" << col << " row " << row << std::endl;
        }
        ++i;
    }
}

std::shared_ptr<LifeRule>
LifeGrid::getRule() const
{
    return m_rule;
}

void
LifeGrid::setRule(const std::shared_ptr<LifeRule>& rule)
{
    m_rule = rule;
}
