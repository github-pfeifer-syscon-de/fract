/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4;  coding: utf-8; -*-  */
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

#include <gtkmm.h>
#include <memory>


class LifeGrid
{
public:
    LifeGrid(uint32_t width, uint32_t height);
    explicit LifeGrid(const LifeGrid& orig) = delete;
    virtual ~LifeGrid() = default;

    size_t getAllocation() const;
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    uint32_t getScaleFactor() const;
    uint32_t getGeneration() const;
    double getComputeTime() const;
    void fill(bool state = false);
    void fillRandom(uint32_t randomness);
    bool nextGen();
    void update(Cairo::RefPtr<Cairo::ImageSurface> m_imageSurface, bool renderWithColor);
    void set(int32_t x, int32_t y, bool set);

private:
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_scaleFactor{2};
    uint32_t m_generation{};
    double m_computeTime{};
    // the storage options are
    // --- enum based on uint8_t
    //  # enum with default, inefficent storage, but fast computation
    //  # separate bool's compact storage, time ~ as above
    std::unique_ptr<std::vector<bool>> m_grid;
    std::unique_ptr<std::vector<bool>> m_changed;
};

class LifeDialog
: public Gtk::Dialog
{
public:
    LifeDialog(BaseObjectType* cobject
            , const Glib::RefPtr<Gtk::Builder>& builder);
    explicit LifeDialog(const LifeDialog& orig) = delete;
    virtual ~LifeDialog() = default;
    void on_response(int response_id) override;
    void apply();
    bool next();
    void random();
    void clear();
    bool drawing_clicked(GdkEventButton* event);
    static void showDialog(Gtk::Application *appl);
protected:
    void createGrid();
    bool drawArea(const Cairo::RefPtr<Cairo::Context>& cr);

private:
    Gtk::ScrolledWindow* m_scroll;
    Gtk::Grid* m_grid;
    Gtk::Button* m_apply;
    Gtk::Button* m_random;
    Gtk::SpinButton* m_width;
    Gtk::SpinButton* m_height;
    Gtk::SpinButton* m_delay;
    Gtk::DrawingArea* m_drawing;
    Gtk::CheckButton* m_colorRender;
    Gtk::SpinButton* m_randomFactor;
    Gtk::Button* m_clear;
    Gtk::Label* m_generation;
    Cairo::RefPtr<Cairo::ImageSurface> m_imageSurface;
    std::shared_ptr<LifeGrid> m_lifeGrid;
    sigc::connection m_timer;

};


