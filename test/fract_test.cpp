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


#include <glibmm.h>
#include <iostream>

#include "fract_config.h"
#include "LifeParser.hpp"
#include "LifeGrid.hpp"

bool
compare(const std::shared_ptr<LifeGrid>& primGrid
      , const std::shared_ptr<LifeGrid>& secGrid)
{
     std::vector<bool> prim = primGrid->getGrid();
     std::vector<bool> secondary = secGrid->getGrid();
     auto diff = std::mismatch(prim.begin(), prim.end(), secondary.begin());
     if (diff.first != prim.end()
     || diff.second != secondary.end()) {
         Life105LifeParser life105parser;
         std::cout << "Prim\n"  << life105parser.exportGrid(primGrid) << std::endl;
         std::cout << "Second\n"  << life105parser.exportGrid(secGrid) << std::endl;

        return false;
     }
     return true;
}

bool
parse()
{
    const int32_t minWidth{16}, minHeight{16};
    std::string rleFormat("#N glider.rle\n"
    "x = 3, y = 3, rule = B3/S23\n"
    "1bo$2bo$3o!\n");
    auto rleParser = LifeParser::getParser(rleFormat);
    if (!rleParser) {
        std::cout << "Rle not recogizied" << std::endl;
        return false;
    }
    Glib::ustring rleMsg;
    rleParser->setMinWidthHeight(minWidth, minHeight);
    auto rleGrid = rleParser->parse(rleFormat, rleMsg);
    if (!rleMsg.empty()) {
        std::cout << "Rle unexpected message " << rleMsg << std::endl;
        return false;
    }

    std::string life105format = "#Life 1.05\n"
        "#D This is a glider.\n"
        "#N\n"
        "#P -1 -1\n"
        ".*.\n"
        "..*\n"
        "***\n";
    auto parser105 = LifeParser::getParser(life105format);
    if (!parser105) {
        std::cout << ".live (105) not recognized" << std::endl;
        return false;
    }
    Glib::ustring life105Msg;
    parser105->setMinWidthHeight(minWidth, minHeight);
    auto life105Grid = parser105->parse(life105format, life105Msg);
    if (!life105Msg.empty()) {
        std::cout << "Life105 unexpected message " << life105Msg << std::endl;
        return false;
    }

    std::string life106format = "#Life 1.06\n"
        "0 -1\n"
        "1 0\n"
        "-1 1\n"
        "0 1\n"
        "1 1\n";
    auto parser106 = LifeParser::getParser(life106format);
    if (!parser106) {
        std::cout << ".live (106) not recognized" << std::endl;
        return false;
    }
    Glib::ustring life106Msg;
    parser106->setMinWidthHeight(minWidth, minHeight);
    auto life106Grid = parser106->parse(life106format, life106Msg);
    if (!life106Msg.empty()) {
        std::cout << "Life106 unexpected message " << life106Msg << std::endl;
        return false;
    }

    //const std::type_info& typeId = typeid(parser.get());
    //std::cout << "typeid " << typeId.name() << std::endl;
    std::string cellFormat = "! Glider\n"
    ".O.\n"
    "..O\n"
    "OOO\n";
    auto cellParser = LifeParser::getParser(cellFormat);
    if (!cellParser) {
        std::cout << ".cell not recogizied" << std::endl;
        return false;
    }
    Glib::ustring cellMsg;
    cellParser->setMinWidthHeight(minWidth, minHeight);
    auto cellGrid = cellParser->parse(cellFormat, cellMsg);
    if (!cellMsg.empty()) {
        std::cout << "cell unexpected message " << cellMsg << std::endl;
        return false;
    }

    if (!compare(rleGrid, life105Grid)) {
        std::cout << "Missmatch rleGrid <-> life105grid" << std::endl;
        return false;
    }
    if (!compare(rleGrid, life106Grid)) {
        // it works but with minimal different pos, so just print ? Minor (will continue)
        std::cout << "Missmatch rleGrid <-> life106grid" << std::endl;
        return false;
    }
    if (!compare(rleGrid, cellGrid)) {
        std::cout << "Missmatch rleGrid <-> cellGrid" << std::endl;
        return false;
    }
    const auto LIFE_ITER{173};
    for (int i = 0; i < LIFE_ITER; ++i) {
        rleGrid->nextGen();
    }
    Life105LifeParser life105parser;
    std::string exp =
        life105parser.getFormatIdentifier() + "\n"
        "................\n"
        "................\n"
        ".*.*............\n"
        "..**............\n"
        "..*.............\n"
        "................\n"
        "................\n"
        "................\n"
        "................\n"
        "................\n"
        "................\n"
        "................\n"
        "................\n"
        "................\n"
        "................\n"
        "................\n";
    auto res = life105parser.exportGrid(rleGrid);
    if (res != exp) {
        std::cout << "Error after "<< LIFE_ITER << " iterations\n" << res
                  << "expected\n" << exp << std::endl;
        return false;
    }
    return true;
}

int
main(int argc, char **argv)
{
    setlocale(LC_ALL, "");      // choose en as parse will be used, and make glib accept u8 const !!!
    Glib::init();
    std::cout << "main " << FRACT_VERSION << std::endl;

    if (!parse()) {
        return 1;
    }
    return 0;
}