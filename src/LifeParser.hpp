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

#include <glibmm.h>
#include <memory>
#include <vector>
#include <list>

class LifeGrid;
class LifeRule;

enum class LifeFileType {
    Rle
    ,Life105
    ,Life106
    ,Cell
};


class LifeParser
{

public:
    LifeParser() = default;
    explicit LifeParser(const LifeParser& orig) = delete;
    virtual ~LifeParser() = default;

    void setMinWidthHeight(int32_t minWidth, int32_t minHeight);
    std::shared_ptr<LifeGrid> parse(const std::string& content, Glib::ustring& msg);
    virtual bool parseLine(std::shared_ptr<LifeGrid>& grid,const std::string& content, Glib::ustring& msg) = 0;
    virtual void reset() = 0;
    virtual bool parseFinal(std::shared_ptr<LifeGrid>& lifeGrid, Glib::ustring& msg) = 0;
    virtual std::string exportGrid(const std::shared_ptr<LifeGrid>& lifeGrid);

    static std::unique_ptr<LifeParser> getParser(const std::string& content);
protected:
    std::pair<int32_t,int32_t> parsePair(const std::string& in, Glib::ustring& msg);
    int32_t m_minWidth{};
    int32_t m_minHeight{};
};

class RleLifeParser
: public LifeParser {
public:
    RleLifeParser() = default;
    explicit RleLifeParser(const RleLifeParser& orig) = delete;
    virtual ~RleLifeParser() = default;

    bool parseLine(std::shared_ptr<LifeGrid>& grid, const std::string& line, Glib::ustring& msg) override;
    void reset() override;
    bool parseFinal(std::shared_ptr<LifeGrid>& lifeGrid, Glib::ustring& msg) override;
    virtual std::string exportGrid(const std::shared_ptr<LifeGrid>& lifeGrid);
    static constexpr auto RLE_LINE_LEN{70};
    static constexpr auto RLE_BLANK_CHAR{'b'};
    static constexpr auto RLE_MARK_CHAR{'o'};
protected:
    void parseAttributes(const std::string& line, Glib::ustring& msg);
    int32_t parseRlePart(const std::string& part, std::vector<bool>& row, Glib::ustring& msg);
    std::string parseRle(const std::string& line, Glib::ustring& msg);
    void addSegment(int32_t n, bool first, std::string& result, std::string::size_type& start);
private:
    int32_t m_width;
    int32_t m_height;
    std::shared_ptr<LifeRule> m_rule;
    std::list<std::vector<bool>> m_segments;
    std::string m_last;
};

class Life105Segment
{
public:
    Life105Segment(const std::pair<int32_t,int32_t>& offs);
    explicit Life105Segment(const Life105Segment& orig) = delete;
    virtual ~Life105Segment() = default;

    void parseLifeLine(const std::string& line, const char markChar);
    int32_t getStartWidth();
    int32_t getSumWidth();
    int32_t getHeight();
    const std::vector<bool>& getSegment();
private:
    std::pair<int32_t,int32_t> m_offs;
    std::vector<bool> m_segment;
};

class Life105LifeParser
: public LifeParser
{
public:
    Life105LifeParser() = default;
    explicit Life105LifeParser(const Life105LifeParser& orig) = delete;
    virtual ~Life105LifeParser() = default;

    bool parseLine(std::shared_ptr<LifeGrid>& grid, const std::string& line, Glib::ustring& msg) override;
    void reset() override;
    bool parseFinal(std::shared_ptr<LifeGrid>& lifeGrid, Glib::ustring& msg) override;
    std::string exportGrid(const std::shared_ptr<LifeGrid>& lifeGrid) override;
    virtual std::string getFormatIdentifier();
protected:
    std::shared_ptr<LifeGrid> parseLifeLines();
    virtual char getCommentChar();
    virtual char getMarkChar();
private:
    std::list<std::unique_ptr<Life105Segment>> m_lifeLines;
    std::pair<int32_t,int32_t> m_offs;

};

class Life106LifeParser
: public LifeParser
{
public:
    Life106LifeParser() = default;
    explicit Life106LifeParser(const Life106LifeParser& orig) = delete;
    virtual ~Life106LifeParser() = default;

    bool parseLine(std::shared_ptr<LifeGrid>& grid, const std::string& line, Glib::ustring& msg) override;
    void reset() override;
    bool parseFinal(std::shared_ptr<LifeGrid>& lifeGrid, Glib::ustring& msg) override;

private:
    std::vector<std::pair<int32_t,int32_t>> m_lifeLines;
};


class CellLifeParser
: public Life105LifeParser
{
public:
    CellLifeParser() = default;
    explicit CellLifeParser(const CellLifeParser& orig) = delete;
    virtual ~CellLifeParser() = default;

    virtual char getCommentChar() override;
    virtual char getMarkChar() override;
    std::string getFormatIdentifier() override;

};