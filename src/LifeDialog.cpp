/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2025 RPf
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

#include "LifeDialog.hpp"

#include <ranges>

LifeGrid::LifeGrid(int32_t width, int32_t height)
: m_width{width}
, m_height{height}
{
    m_grid = std::make_unique<std::vector<bool>>(getAllocation(), false);
    m_changed = std::make_unique<std::vector<bool>>(getAllocation(), false);
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
    std::fill(m_grid->begin(), m_grid->end(), state);
    std::fill(m_changed->begin(), m_changed->end(), state);
    m_generation = 0;
}

void
LifeGrid::fillRandom(int32_t randomness)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    auto mid = randomness / 2u;
    for (uint32_t i = 0; i < m_grid->size(); ++i) {
        auto val = rng() % randomness;
        (*m_grid)[i] = (val == mid);
        (*m_changed)[i] = (val == mid);
    }
    m_generation = 0;
}

// remember sums avoid multiple index eval
//   the calculation of prepared sums reduces times from ~3ms to ~2ms
std::unique_ptr<std::vector<int32_t>>
LifeGrid::getRowCount(int32_t row)
{
    auto rowCnt = std::make_unique<std::vector<int32_t>>(m_width);
    const auto rowsOffs = row * m_width;
    int32_t prevCellCnt = (*m_grid)[rowsOffs + (m_width - 1)] ? 1 : 0;
    int32_t thisCellCnt = (*m_grid)[rowsOffs + 0] ? 1 : 0;
    for (int32_t cell = 0; cell < m_width; ++cell) {
        const auto nextCell = cell < m_width - 1 ? cell + 1 : 0;
        int32_t nextCellCnt = (*m_grid)[rowsOffs + nextCell] ? 1 : 0;
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
        const auto rowsOffs = row * m_width;
        std::unique_ptr<std::vector<int32_t>> nextRowCnt;
        if (row < m_height - 1) {
            nextRowCnt = getRowCount(row + 1);
        }
        else {
            nextRowCnt = std::move(zeroRowCnt);
        }
        for (int32_t cell = 0; cell < m_width; ++cell) {
            bool set;
            auto cnt = (*prevRowCnt)[cell] + (*thisRowCnt)[cell] + (*nextRowCnt)[cell];
            if ((*m_grid)[rowsOffs + cell]) {
                set = cnt >= 3 && cnt <= 4;     // original values are 2,3 but here we sum the cell we are on as well
                (*m_grid)[rowsOffs + cell] = set;
                (*m_changed)[rowsOffs + cell] = !set;
            }
            else {
                set = cnt == 3;
                (*m_grid)[rowsOffs + cell] = set;
                (*m_changed)[rowsOffs + cell] = set;
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
LifeGrid::update(Cairo::RefPtr<Cairo::ImageSurface> imageSurface, bool renderWithColor)
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
        const auto cellOffs = row * m_width;
        for (int32_t cell = 0; cell < m_width; ++cell) {
            auto cellVal = (*m_grid)[cellOffs + cell];
            auto cellChanged = (*m_changed)[cellOffs + cell];
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
        const auto rowsOffs = iy * m_width;
        (*m_grid)[rowsOffs + ix] = set;
        (*m_changed)[rowsOffs + ix] = true;
        m_generation = 0;
     }
}


LifeDialog::LifeDialog(BaseObjectType* cobject
                    , const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject)
{
    builder->get_widget("width", m_width);
    builder->get_widget("height", m_height);
    builder->get_widget("delay", m_delay);
    builder->get_widget("grid", m_grid);
    builder->get_widget("buttonApply", m_apply);
    builder->get_widget("buttonRandom", m_random);
    builder->get_widget("scroll", m_scroll);
    builder->get_widget("drawing", m_drawing);
    builder->get_widget("colorRender", m_colorRender);
    builder->get_widget("randomFactor", m_randomFactor);
    builder->get_widget("buttonClear", m_clear);
    builder->get_widget("generation", m_generation);

    m_apply->signal_clicked().connect(
        sigc::mem_fun(*this, &LifeDialog::apply));
    m_random->signal_clicked().connect(
        sigc::mem_fun(*this, &LifeDialog::random));
    m_clear->signal_clicked().connect(
        sigc::mem_fun(*this, &LifeDialog::clear));
    m_drawing->signal_draw().connect(
        sigc::mem_fun(*this, &LifeDialog::drawArea));
    m_drawing->add_events(Gdk::EventMask::BUTTON_PRESS_MASK
                        | Gdk::EventMask::BUTTON_RELEASE_MASK
                        | Gdk::EventMask::BUTTON_MOTION_MASK);
    m_drawing->signal_button_press_event().connect(
        sigc::mem_fun(*this, &LifeDialog::drawing_clicked));
    m_drawing->signal_button_release_event().connect(
        sigc::mem_fun(*this, &LifeDialog::drawing_clicked));
    m_drawing->signal_motion_notify_event().connect(
        sigc::mem_fun(*this, &LifeDialog::drawing_motion));
}

void
LifeDialog::on_response(int response_id)
{
    if (m_timer) {
        m_timer.disconnect();   // stop update
    }
    Gtk::Dialog::on_response(response_id);
}

bool
LifeDialog::drawing_clicked(GdkEventButton* event)
{
    //std::cout << "x " << event->x << " y " << event->y << " btn " << event->button << std::endl;
    m_mouseButton = event->button;
    if (m_mouseButton != 0) {
        createGrid();
        m_lifeGrid->set(event->x, event->y, event->button == GDK_BUTTON_PRIMARY);
        m_lifeGrid->update(m_imageSurface, m_colorRender->get_active());
        m_drawing->queue_draw();
    }
    return true;    // event handled
}

bool
LifeDialog::drawing_motion(GdkEventMotion* event)
{
    if (m_mouseButton != 0) {
        createGrid();
        m_lifeGrid->set(event->x, event->y, m_mouseButton  == GDK_BUTTON_PRIMARY);
        m_lifeGrid->update(m_imageSurface, m_colorRender->get_active());
        m_drawing->queue_draw();
    }
    return true;    // event handled
}

void
LifeDialog::apply()
{
    if (m_timer) {  // toggle on off
        m_timer.disconnect();
        //m_apply->set_label("Apply"); removes image
        return;
    }
    createGrid();
    int32_t delay = m_delay->get_value_as_int();
    m_timer = Glib::signal_timeout().connect(sigc::mem_fun(*this, &LifeDialog::next), delay);
    //m_apply->set_label("Stop");
}

bool
LifeDialog::drawArea(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    if (m_imageSurface) {
        cr->set_source(m_imageSurface, 0.0, 0.0);
        cr->paint();
    }
    else {
        cr->set_source_rgb(0.0, 0.0, 0.0);
        cr->rectangle(0,0, allocation.get_width(), allocation.get_height());
        cr->fill();
    }
    return true;
}

bool
LifeDialog::next()
{
    bool next = m_lifeGrid->nextGen();
    m_lifeGrid->update(m_imageSurface, m_colorRender->get_active());
    m_drawing->queue_draw();
    m_generation->set_text(Glib::ustring::sprintf("Generation %u time\u2205 %.1lfms"
        , m_lifeGrid->getGeneration(), m_lifeGrid->getComputeTime() * 1000.0));
    return next;
}

void
LifeDialog::random()
{
    if (m_timer) {
        m_timer.disconnect();   // stop update
    }
    createGrid();
    m_lifeGrid->fillRandom(m_randomFactor->get_value_as_int());
    m_lifeGrid->update(m_imageSurface, m_colorRender->get_active());
    m_drawing->queue_draw();
}

void
LifeDialog::clear()
{
    if (m_timer) {
        m_timer.disconnect();   // stop update
    }
    createGrid();
    m_lifeGrid->fill();
    m_lifeGrid->update(m_imageSurface, m_colorRender->get_active());
    m_drawing->queue_draw();
}

void
LifeDialog::createGrid()
{
    int32_t width = m_width->get_value_as_int();
    int32_t height = m_height->get_value_as_int();
    if (!m_lifeGrid
      || m_lifeGrid->getWidth() != width
      || m_lifeGrid->getHeight() != height) {
        m_lifeGrid  = std::make_shared<LifeGrid>(width, height);
        auto scaleFactor = m_lifeGrid->getScaleFactor();
        auto reqWidth{static_cast<int>(width * scaleFactor)};
        auto reqHeight{static_cast<int>(height * scaleFactor)};
        m_imageSurface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, reqWidth, reqHeight);
        m_drawing->set_size_request(reqWidth, reqHeight);
        set_size_request(reqWidth + 250, reqHeight + 50);   // scale dialog to view
    }
}

void
LifeDialog::showDialog(Gtk::Application *appl)
{
    auto refBuilder = Gtk::Builder::create();
    try {
        refBuilder->add_from_resource(appl->get_resource_base_path() + "/life-dlg.ui");
        LifeDialog* lifeDialog{};
        refBuilder->get_widget_derived("LifeDialog", lifeDialog);
        lifeDialog->run();
        delete lifeDialog;  // delete top level
    }
    catch (const Glib::Error& ex) {
        std::cerr << "LifeDialog::show(): " << ex.what() << std::endl;
    }
}