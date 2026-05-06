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
#include <algorithm>

#include "LifeGrid.hpp"
#include "LifeParser.hpp"

// suggested by https://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string
static
std::string
trim(const std::string& str,
     const std::string& whitespace = " \t\n")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

static
std::string
reduce(const std::string& str,
       const std::string& fill = " ",
       const std::string& whitespace = " \t")
{
    // trim first
    auto result = trim(str, whitespace);

    // replace sub ranges
    auto beginSpace = result.find_first_of(whitespace);
    while (beginSpace != std::string::npos) {
        const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
        const auto range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        const auto newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}

static std::string
toLower(const std::string in)
{
    std::string out(in.size(), ' ');
    std::ranges::transform(in, out.begin(), ::tolower);
    return out;
}

void
LifeParser::setMinWidthHeight(int32_t minWidth, int32_t minHeight)
{
    m_minWidth = minWidth;
    m_minHeight = minHeight;
}

std::shared_ptr<LifeGrid>
LifeParser::parse(const std::string& content, Glib::ustring& msg)
{
    reset();
    std::shared_ptr<LifeGrid> lifeGrid;
    std::stringstream strStream(content);
    std::string line;
    bool success{false};
    while(std::getline(strStream, line, '\n')) {
        line = trim(line, " \t\r\n");
        if (!line.empty() ) {
            success = parseLine(lifeGrid, line, msg);
            if (!success) {
                break;
            }
        }
    }
    if (success) {
        parseFinal(lifeGrid, msg);
    }
    return lifeGrid;
}

std::pair<int32_t,int32_t>
LifeParser::parsePair(const std::string& in, Glib::ustring& msg)
{
    int32_t x{},y{};
    try {
        std::string::size_type pos;
        x = std::stoi(in, &pos);
        auto remain = in.substr(pos);
        y = std::stoi(remain, &pos);
        if (pos != remain.size()) {
            msg += Glib::ustring::sprintf("LifeParser line \"%s\" not recognized info after number.\n", in);
        }
    }
    catch (std::invalid_argument const& ex) {
        msg += Glib::ustring::sprintf("LifeParser line \"%s\" not parsable numbers.\n", in);
    }
    return std::make_pair(x,y);
}

std::string
LifeParser::exportGrid(const std::shared_ptr<LifeGrid>& lifeGrid)
{
    return "";      // leave this to concrete implementations
}

std::unique_ptr<LifeParser>
LifeParser::getParser(const std::string& content)
{
    std::unique_ptr<LifeParser> parser;
    int32_t rleBOLines{};
    int32_t rleAssignLines{};   // rel assign = ,
    int32_t life105Lines{};
    int32_t life106Lines{};
    int32_t dotStarLines{};
    int32_t dotOLines{};
    std::stringstream strStream(content);
    std::string line;
    while(std::getline(strStream, line, '\n')) {
        line = trim(line, " \t\r\n");
        if (!line.empty() ) {
            if (line == "#Life 1.05") {
                ++life105Lines;
            }
            if (line == "#Life 1.06") {
                ++life106Lines;
            }
            if (line.find('=') != line.npos
              && line.find(',') != line.npos) {
                ++rleAssignLines;
            }
            if (line.find('b') != line.npos
             && line.find('o') != line.npos) {
                ++rleBOLines;
            }
            if (line.find('*') != line.npos
             && line.find('.') != line.npos) {
                ++dotStarLines;
            }
            if (line.find('O') != line.npos
             && line.find('.') != line.npos) {
                ++dotOLines;
            }
        }
    }
    if (rleAssignLines > 0 && rleBOLines > 0) {
        parser = std::make_unique<RleLifeParser>();
    }
    else if (life105Lines > 0 && dotStarLines > 0) {
        parser = std::make_unique<Life105LifeParser>();
    }
    else if (life106Lines > 0) {
        parser = std::make_unique<Life106LifeParser>();
    }
    else if (dotOLines > 0) {
        parser = std::make_unique<CellLifeParser>();
    }

    return parser;
}

// Attributes used by .rle files
// e.g. "x = 28, y = 28, rule = B3/S23\n"
void
RleLifeParser::parseAttributes(const std::string& line, Glib::ustring& msg)
{
    std::shared_ptr<LifeGrid> lifeGrid;
    std::string::size_type last{};
    std::string rule;
    do {
        auto comPos = line.find(',', last);
        if (comPos == line.npos) {
            comPos = line.size();
        }
        auto assign = line.substr(last, comPos - last);
        assign = reduce(assign, "");
        auto eqPos = assign.find('=');
        if (eqPos != assign.npos) {
            assign = toLower(assign);
            const std::string name = assign.substr(0, eqPos);
            ++eqPos;
            const std::string value = assign.substr(eqPos);
            try {
                if (name == "x") {
                    m_width = std::stoi(value);
                }
                else if (name == "y") {
                    m_height = std::stoi(value);
                }
                else if (name == "rule") {
                    auto dynRule = std::make_shared<DynamicLifeRule>(value);
                    auto ruleMsg = dynRule->getMessage();
                    if (!ruleMsg.empty()) {
                        msg += ruleMsg + "\n";
                    }
                    else {
                        m_rule = dynRule;
                    }
                }
                else {
                    msg += Glib::ustring::sprintf("Assign %s not used\n", assign);
                }
            }
            catch (std::invalid_argument const& ex) {
                msg += Glib::ustring::sprintf("Rle attribute name \"%s\" value \"%s\" not a parsable number\n", name, value);
            }
        }
        last = comPos + 1;
    } while (last <  line.size());
    if (!m_rule) {
        m_rule = std::make_shared<LifeRule23>();
        msg += Glib::ustring::sprintf("Using default rule\n");
    }
}

// places data for one output line into row
int32_t
RleLifeParser::parseRlePart(const std::string& part, std::vector<bool>& row, Glib::ustring& msg)
{
    std::string::size_type pos{};
    auto col = row.begin();
    do {
        int32_t n{1};
        try {
            std::string::size_type chrs{};
            n = std::stoi(part.substr(pos), &chrs);
            pos += chrs;
        }
        catch (std::invalid_argument const& ex) { // happens if not starting with number
        }
        if (part[pos] == RLE_BLANK_CHAR) {  // was already initialized as empty...
            col += n;
            ++pos;
        }
        else if (part[pos] == RLE_MARK_CHAR) {
            auto max = std::distance(col, row.end());
            if (n > max) {
                if (msg.length() < 1024) {
                    msg += Glib::ustring::sprintf("Index %d exceeds avail %d\n",
                        std::distance(row.begin(), col) + n, row.size());
                }
                n = max;
            }
            std::fill(col, col + n, true);
            col += n;
            ++pos;
        }
        else if (part[pos] == '!') {
            return 0;
        }
        else if (pos >= part.size()) {  // just number -> added line
            return n;
        }
        else {
            std::cout <<  "RleLifeParser::parseRlePart unknown part " << part << " pos " << pos << std::endl;
            break;
        }
    } while (pos < part.size());
    return 1;   // default skip one line
}

// Parse a rle data line
//   here we split the $ parts and return an incomplete last part
// "15bo$13b3o$12bo$12b2o2$bo$obo7b2o$bo7b3o3b2o$10b2o3b2o3$19b2o$10b2o7b2o5b2o$b\n"
std::string
RleLifeParser::parseRle(const std::string& line, Glib::ustring& msg)
{
    std::string::size_type last{};
    do {
        auto doPos = line.find('$', last);
        if (doPos == line.npos) {
            auto exPos = line.find('!', last);
            if (exPos == line.npos) {
                return line.substr(last);
            }
            doPos = exPos + 1;  // exclamation is delimiter for last part
        }
        auto part = line.substr(last, doPos - last);
        std::vector<bool> row(m_width, false);
        auto n = parseRlePart(part, row, msg);
        m_segments.emplace_back(std::move(row));
        //std::cout << "part " << part << " n " << n << std::endl;
        for (int32_t i = 1; i < n; ++i) {   // fill only the extras
            m_segments.emplace_back(
                std::move(std::vector<bool>(m_width, false)));
        }
        last = doPos + 1;
    } while (last < line.size());
    return "";
}

void
RleLifeParser::reset()
{
    m_width = 0;
    m_height = 0;
    m_rule.reset();
    m_segments.clear();
    m_last = "";
}


bool
RleLifeParser::parseLine(std::shared_ptr<LifeGrid>& lifeGrid, const std::string& line, Glib::ustring& msg)
{
    if (line[0] != '#') {
        auto comPos = line.find(',');
        if (comPos != line.npos) {
            parseAttributes(line, msg);
        }
        else if (line.find(RLE_MARK_CHAR) != line.npos
              ||line.find(RLE_BLANK_CHAR) != line.npos) {
            if (m_width == 0 || m_height == 0) {
                msg += "Attributes not recognized within file.\n";
                return false;
            }
            auto rleLine = line;
            if (!m_last.empty()) {
                rleLine = m_last + rleLine;
                m_last.clear();
            }
            m_last = parseRle(rleLine, msg);
        }
    }
    return true;
}

bool
RleLifeParser::parseFinal(std::shared_ptr<LifeGrid>& lifeGrid, Glib::ustring& msg)
{
    auto width = std::max(m_width, m_minWidth);
    auto height = std::max(m_height, m_minHeight);
    auto width_offs = (width - m_width) / 2;
    auto height_offs = (height - m_height) / 2;
    int32_t row{height_offs};
    auto grid = std::make_unique<std::vector<bool>>(width * height, false);
    for (auto& segm : m_segments) {
        auto gridPos = grid->begin() + row * width + width_offs;
        std::copy(segm.begin(), segm.end(), gridPos);
        ++row;
    }
    lifeGrid = std::make_shared<LifeGrid>(width, height, grid);
    if (m_rule) {
        lifeGrid->setRule(m_rule);
    }
    return true;
}

void
RleLifeParser::addSegment(int32_t n
        , bool first
        , std::string& result
        , std::string::size_type& start)
{
    std::string part;
    if (n > 1) {
        part = std::to_string(n);
    }
    part += first ? RLE_MARK_CHAR : RLE_BLANK_CHAR;
    auto newSize{result.size() + part.size()};
    if (newSize - start >= RLE_LINE_LEN) {
        result += "\n";
        start = newSize;
    }
    result += part;
}

std::string
RleLifeParser::exportGrid(const std::shared_ptr<LifeGrid>& lifeGrid)
{
    // does not reduce larger figures to needed size...
    // the line size limit is not exactly followed
    std::string result;
    if (lifeGrid) {
        result.reserve((lifeGrid->getWidth()) * lifeGrid->getHeight() / 2);
        result += Glib::ustring::sprintf("x = %d, y = %d, rule = %s\n", lifeGrid->getWidth(), lifeGrid->getHeight(), lifeGrid->getRule()->getName());
        auto start = result.size();
        auto grid = lifeGrid->getGrid();
        std::vector<bool> emptyLine(lifeGrid->getHeight(), false);
        for (int32_t row = 0; row < lifeGrid->getHeight(); ++row) {
            auto colStart = grid.begin() + (row * lifeGrid->getWidth());
            auto colEnd = grid.begin() + ((row + 1) * lifeGrid->getWidth());
            auto found = std::ranges::find(colStart, colEnd, true);
            emptyLine[row] = (found == colEnd);
        }
        for (int32_t row = 0; row < lifeGrid->getHeight(); ++row) {
            auto emptyStart = emptyLine.begin() + row;
            auto nonEmpty = std::ranges::find(emptyStart, emptyLine.end(), false);
            if (nonEmpty != emptyStart) {
                int32_t n = std::distance(emptyStart, nonEmpty);
                result += std::to_string(n);
                row += n - 1;   // -1 since the loop will increment as well
            }
            else {
                auto colStart = grid.begin() + (row * lifeGrid->getWidth());
                auto colEnd = grid.begin() + ((row + 1) * lifeGrid->getWidth());
                while (colStart != colEnd) {
                    auto value = *colStart;
                    auto found = std::ranges::find(colStart, colEnd, !value);
                    auto n = std::distance(colStart, found);
                    addSegment(n, value, result, start);
                    colStart = found;
                }
            }
            result += row < lifeGrid->getHeight() - 1 ? '$' : '!';
        }
    }
    return result;
}

char
Life105LifeParser::getCommentChar()
{
    return '#';
}


char
Life105LifeParser::getMarkChar()
{
    return '*';
}

Life105Segment::Life105Segment(const std::pair<int32_t,int32_t>& offs)
: m_offs{offs}
{
}

// parse simple formats
// e.g. .cell ".........O....OO.............."
//      .life ".........*....**"
void
Life105Segment::parseLifeLine(const std::string& line, const char markChar )
{
    m_segment.reserve(line.size());
    for (std::string::size_type i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == markChar) {
            m_segment.push_back(true);
        }
        else if (c == '.') {
            m_segment.push_back(false);
        }
    }
}

int32_t
Life105Segment::getStartWidth()
{
    return std::max(m_offs.first, 0);  // ignore negative offsets as we center pieces by ourself
}

int32_t
Life105Segment::getSumWidth()
{
    return getStartWidth() + m_segment.size();
}

int32_t
Life105Segment::getHeight()
{
    return std::max(m_offs.second, 0);  // ignore negative offsets as we center pieces by ourself
}

const std::vector<bool>&
Life105Segment::getSegment() {
    return m_segment;
}

void
Life105LifeParser::reset()
{
    m_lifeLines.clear();
    m_offs = std::make_pair(0,0);
}

bool
Life105LifeParser::parseLine(std::shared_ptr<LifeGrid>& lifeGrid, const std::string& line, Glib::ustring& msg)
{
    const char commentChar = getCommentChar();
    if (line.length() > 3 && line.substr(0,2) == "#R") {    // #N will be ignored here, but will add in final
        std::string rule = line.substr(2);
        auto dynRule = std::make_shared<DynamicLifeRule>(rule);
        auto ruleMsg = dynRule->getMessage();
        if (ruleMsg.empty()) {
            m_rule = dynRule;
        }
        else {
            msg += ruleMsg+"\n";
        }
    }
    else if (line.length() > 3 && line.substr(0,2) == "#P") {
        m_offs = parsePair(line.substr(2), msg);
    }
    else if (line[0] != commentChar) {
        const char markChar = getMarkChar();
        if (line.find(markChar) != line.npos
          ||line.find('.') != line.npos) {
            auto lifeLine = std::make_unique<Life105Segment>(m_offs);
            lifeLine->parseLifeLine(line, getMarkChar());
            m_lifeLines.emplace_back(std::move(lifeLine));
        }
        else if (!line.empty()) {
            msg += Glib::ustring::sprintf("Life105LifeParser unknown line \"%s\".\n", line);
        }
    }
    return true;
}

bool
Life105LifeParser::parseFinal(std::shared_ptr<LifeGrid>& lifeGrid, Glib::ustring& msg)
{
    if (!m_lifeLines.empty()) {
        lifeGrid = parseLifeLines();
        return true;
    }
    return false;
}

// since the simple formats have no size information
//   we convert them into our grid at the end.
std::shared_ptr<LifeGrid>
Life105LifeParser::parseLifeLines()
{
    int32_t maxWidth{};
    int32_t minWidth{1024};
    int32_t row{};
    int32_t maxHeight{};
    int32_t minHeight{1024};
    for (auto& segment : m_lifeLines) {
        minWidth = std::min(minWidth, segment->getStartWidth());
        maxWidth = std::max(maxWidth, segment->getSumWidth());
        ++row;
        auto height = segment->getHeight() + row;
        minHeight = std::min(minHeight, height);
        maxHeight = std::max(maxHeight, height);
    }
    auto diffWidth = maxWidth - minWidth;
    auto diffHeight = maxHeight - minHeight;
    auto width = std::max(diffWidth, m_minWidth);
    auto height = std::max(diffHeight, m_minHeight);
    auto widthOffs = -minWidth + (width - diffWidth) / 2;
    auto heightOffs = -minHeight + (height -  diffHeight) / 2;
    std::shared_ptr<LifeGrid> lifeGrid = std::make_shared<LifeGrid>(width, height);
    row = 0;
    for (auto& line : m_lifeLines) {
        int32_t col{};
        for (bool set : line->getSegment()) {
            if (set) {
                lifeGrid->setCell(widthOffs + col,  heightOffs + row, 1, true);
            }
            ++col;
        }
        ++row;
    }
    if (!m_rule) {
        m_rule = std::make_shared<LifeRule23>();
    }
    lifeGrid->setRule(m_rule);
    return lifeGrid;
}


std::string
Life105LifeParser::getFormatIdentifier()
{
    return "#Life 1.05";
}

std::string
Life105LifeParser::exportGrid(const std::shared_ptr<LifeGrid>& lifeGrid)
{
    std::string result;
    if (lifeGrid) {
        result.reserve(16 + (lifeGrid->getWidth() + 2) * lifeGrid->getHeight());
        result += getFormatIdentifier() + "\n";   // use the visually obvious life 1.05 as default
        auto grid = lifeGrid->getGrid();
        for (int32_t row = 0; row < lifeGrid->getHeight(); ++row) {
            auto rowOffs = row * lifeGrid->getWidth();
            for (int32_t col = 0; col < lifeGrid->getWidth(); ++col) {
                result += grid[rowOffs + col] ? getMarkChar() : '.';
            }
            result += '\n';
        }
    }
    return result;
}


bool
Life106LifeParser::parseLine(std::shared_ptr<LifeGrid>& grid, const std::string& iline, Glib::ustring& msg)
{
    auto line = reduce(iline);
    if (line[0] != '#') {
        auto m_val = parsePair(line, msg);
        m_lifeLines.push_back(m_val);
    }

    return true;
}

void
Life106LifeParser::reset()
{
    m_lifeLines.clear();
}

bool
Life106LifeParser::parseFinal(std::shared_ptr<LifeGrid>& lifeGrid, Glib::ustring& msg)
{
    if (!m_lifeLines.empty()) {
        std::pair max{0, 0};
        std::pair min{1024,1024};
        for (auto& pair : m_lifeLines) {
            min.first = std::min(min.first, pair.first);
            min.second = std::min(min.second, pair.second);
            max.first = std::max(max.first, pair.first);
            max.second = std::max(max.second, pair.second);
        }
        auto gridWidth = max.first - min.first + 1; // if there are 2 steps inbetween the width is 3
        auto gridHeight = max.second - min.second + 1;
        auto width = std::max(gridWidth, m_minWidth);
        auto height = std::max(gridHeight, m_minHeight);
        auto offs_x = -min.first + (width - gridWidth) / 2;
        auto offs_y = -min.second + (height - gridHeight) / 2;
        lifeGrid = std::make_shared<LifeGrid>(width, height);
        for (auto& pair : m_lifeLines) {
            auto x{offs_x + pair.first};
            auto y {offs_y + pair.second};
            lifeGrid->setCell(x, y, 1, true);
        }
        return true;
    }
    return false;
}

char
CellLifeParser::getCommentChar()
{
    return '!';
}

char
CellLifeParser::getMarkChar()
{
    return 'O';
}

std::string
CellLifeParser::getFormatIdentifier()
{
    return "! .cell format";
}