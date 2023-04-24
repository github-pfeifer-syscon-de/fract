/*
 * Copyright (C) 2017 rpf
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
#include <list>
#include <atomic>
#include <memory>
#include <thread>

#include "Param.h"
#include "Worker.h"

class FractView : public Gtk::DrawingArea {
public:
    FractView(Gtk::Window &_parent, std::shared_ptr<Param> param, Gtk::Application *appl);
    virtual ~FractView();
    void stop_workers(bool waitComplete);
    guint32 get_row();
    Cairo::RefPtr<Cairo::ImageSurface> get_imagesurface();
    std::shared_ptr<Param> get_param();
    void notifyRow(guint row);
    void on_action_about();
protected:
    std::shared_ptr<Param> m_param;
    Cairo::RefPtr<Cairo::ImageSurface> m_pixmap;
private:
    Glib::Dispatcher m_Dispatcher; // used for redraw notification
    mutable std::mutex m_Mutex; // serialize redraw area building and drawing
    Cairo::RectangleInt *m_selectedRect;
    Gtk::Window &m_parent;
    Gtk::Application *m_appl;
    Worker<double> **m_worker;
    guint32 m_proc; // cores or processors
    std::atomic<unsigned int> m_row; // atomic is expected to be faster than a mutex
    guint32 m_redraw_start;
    guint32 m_redraw_end;
    gboolean m_redraw_pending;
    std::list<std::shared_ptr<Param>> m_lastParam;
    //std::list<gpointer>      m_ChildFract;
    Gtk::Menu *build_popup();
    bool on_motion_notify_event(GdkEventMotion* event);
    bool on_button_release_event(GdkEventButton* event);
    bool on_button_press_event(GdkEventButton* event);
    bool on_draw(const Cairo::RefPtr<Cairo::Context> &cri);
    void on_notification_from_worker_thread();
    void on_menu_save();
    void on_menu_open();
    void on_menu_julia();
    void on_menu_last(guint n);
    void start_new();
    void fill();
    void reinit_redraw();
    void fractWinClose(gpointer fract);
    void on_menu_param();
    void save_png(Glib::ustring filename);
};

