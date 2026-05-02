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

LifeGrid::LifeGrid(uint32_t width, uint32_t height)
: m_width{width}
, m_height{height}
{
    m_grid = std::make_unique<std::vector<bool>>(getAllocation(), false);
    m_changed = std::make_unique<std::vector<bool>>(getAllocation(), false);
    m_scaleFactor =
        m_width >= 512 || m_height >= 512 ? 1 :
        m_width >= 256 ? 2 : 3;
}

size_t
LifeGrid::getAllocation() const
{
    return m_width * m_height;
}

uint32_t
LifeGrid::getWidth() const
{
    return m_width;
}

uint32_t
LifeGrid::getHeight() const
{
    return m_height;
}

uint32_t
LifeGrid::getScaleFactor() const
{
    return m_scaleFactor;
}

uint32_t
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
LifeGrid::fillRandom(uint32_t randomness)
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

bool
LifeGrid::nextGen()
{
    auto nextGrid = std::make_unique<std::vector<bool>>(getAllocation(), false);
    const auto start{std::chrono::steady_clock::now()};
    bool anySet{};
    for (uint32_t row = 0; row < m_height; ++row) {
        const auto rowsOffs = row * m_width;
        const auto prevRow = row > 0 ? row - 1 : m_height - 1;    // wrap surface
        const auto prevRowOffs = prevRow * m_width;
        const auto nextRow = row < m_height - 1 ? row + 1 : 0;
        const auto nextRowOffs = nextRow * m_width;
        for (uint32_t cell = 0; cell < m_width; ++cell) {
            uint32_t cnt{};
            const auto prevCell = cell > 0 ? cell - 1 : m_width - 1;
            const auto nextCell = cell < m_width - 1 ? cell + 1 : 0;
            if ((*m_grid)[prevRowOffs + prevCell]) {
                ++cnt;
            }
            if ((*m_grid)[prevRowOffs + cell]) {
                ++cnt;
            }
            if ((*m_grid)[prevRowOffs + nextCell]) {
                ++cnt;
            }

            if ((*m_grid)[rowsOffs + prevCell]) {
                ++cnt;
            }
            if ((*m_grid)[rowsOffs + nextCell]) {
                ++cnt;
            }

            if ((*m_grid)[nextRowOffs + prevCell]) {
                ++cnt;
            }
            if ((*m_grid)[nextRowOffs + cell]) {
                ++cnt;
            }
            if ((*m_grid)[nextRowOffs + nextCell]) {
                ++cnt;
            }
            bool set;
            if ((*m_grid)[rowsOffs + cell]) {
                set = cnt >= 2 && cnt <= 3;
                (*nextGrid)[rowsOffs + cell] = set;
                (*m_changed)[rowsOffs + cell] = !set;
            }
            else {
                set = cnt == 3;
                (*nextGrid)[rowsOffs + cell] = set;
                (*m_changed)[rowsOffs + cell] = set;
            }
            anySet |= set;
        }
    }
    m_grid = std::move(nextGrid);
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
    const auto rowStride = (imageSurface->get_stride() / sizeof(uint32_t));
    for (uint32_t row = 0; row < m_height; ++row) {
        const auto cellOffs = row * m_width;
        for (uint32_t cell = 0; cell < m_width; ++cell) {
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
            for (uint32_t j = 0; j < m_scaleFactor; ++j) {
                auto rowPixOffs = data + ((row * m_scaleFactor + j) * rowStride) + cell * m_scaleFactor;
                for (uint32_t i = 0; i < m_scaleFactor; ++i) {
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
LifeGrid::set(int32_t x, int32_t y, bool set)
{
    const auto rowsOffs = y * m_width;
    (*m_grid)[rowsOffs + x] = set;
    (*m_changed)[rowsOffs + x] = true;
    m_generation = 0;
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

    m_apply->signal_clicked().connect(sigc::mem_fun(*this, &LifeDialog::apply));
    m_random->signal_clicked().connect(sigc::mem_fun(*this, &LifeDialog::random));
    m_clear->signal_clicked().connect(sigc::mem_fun(*this, &LifeDialog::clear));
    m_drawing->signal_draw().connect(sigc::mem_fun(*this, &LifeDialog::drawArea));
    m_drawing->add_events(Gdk::EventMask::BUTTON_PRESS_MASK);
    m_drawing->signal_button_press_event().connect(sigc::mem_fun(*this, &LifeDialog::drawing_clicked));
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
    std::cout << "x " << event->x << " y " << event->y << " btn " << event->button << std::endl;
    createGrid();
    auto ix = static_cast<int32_t>(event->x / static_cast<double>(m_lifeGrid->getScaleFactor()));
    auto iy = static_cast<int32_t>(event->y / static_cast<double>(m_lifeGrid->getScaleFactor()));
    if (ix >= 0 && ix < static_cast<int32_t>(m_lifeGrid->getWidth())
     && iy >= 0 && iy < static_cast<int32_t>(m_lifeGrid->getHeight())) {
        m_lifeGrid->set(ix, iy, event->button == GDK_BUTTON_PRIMARY);
        m_lifeGrid->update(m_imageSurface, m_colorRender->get_active());
        m_drawing->queue_draw();
        //return false;
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
    uint32_t delay = static_cast<uint32_t>(m_delay->get_value_as_int());
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
    m_generation->set_text(Glib::ustring::sprintf("Generation %u time %.1lfms"
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
    m_lifeGrid->fillRandom(static_cast<uint32_t>(m_randomFactor->get_value_as_int()));
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
    uint32_t width = static_cast<uint32_t>(m_width->get_value_as_int());
    uint32_t height = static_cast<uint32_t>(m_height->get_value_as_int());
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