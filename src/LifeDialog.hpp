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

#include "LifeGrid.hpp"
#include "LifeQueryDialog.hpp"

class LifeDialog
: public Gtk::Dialog
, public ResultCallback
{
public:
    LifeDialog(BaseObjectType* cobject
            , const Glib::RefPtr<Gtk::Builder>& builder
            , Gtk::Application *appl);
    explicit LifeDialog(const LifeDialog& orig) = delete;
    virtual ~LifeDialog() = default;
    void on_response(int response_id) override;
    void apply();
    bool next();
    void random();
    void clear();
    void open();
    std::string getDefaultSrc() override;
    void notify(const std::string& value) override;

    bool drawing_clicked(GdkEventButton* event);
    bool drawing_motion(GdkEventMotion* event);
    static void showDialog(Gtk::Application *appl);
    // provide a example to find more ...
    static constexpr auto EXAMPLE_ADDRESS{"https://www.conwaylife.com/patterns/10enginecordership.rle"};
protected:
    void createGrid();
    bool drawArea(const Cairo::RefPtr<Cairo::Context>& cr);
    void show_error(const Glib::ustring& msg, Gtk::MessageType type = Gtk::MessageType::MESSAGE_ERROR);
    void adjustImageSize();
    void updateMouse(double mx, double my, guint button);

private:
    Gtk::Application *m_appl;
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
    Gtk::Entry* m_rule;
    Gtk::Button* m_open;
    Cairo::RefPtr<Cairo::ImageSurface> m_imageSurface;
    std::shared_ptr<LifeGrid> m_lifeGrid;
    sigc::connection m_timer;
    guint m_mouseButton{};
};


