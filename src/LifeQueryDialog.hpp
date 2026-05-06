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

#include <gtkmm.h>
#include <libsoup/soup.h>
#include <memory>

#include "LifeParser.hpp"

class LifeGrid;

class ResultCallback
{
public:
    virtual ~ResultCallback() = default;

    virtual std::string getDefaultSrc() = 0;
    virtual void notify(const std::string& value) = 0;
};

class LifeQueryDialog
: public Gtk::Dialog
{
public:
    LifeQueryDialog(BaseObjectType* cobject
                , const Glib::RefPtr<Gtk::Builder>& builder
                , ResultCallback* resultCallback
                , const std::shared_ptr<LifeGrid>& lifeGrid);
    explicit LifeQueryDialog(const LifeQueryDialog& other) = delete;
    virtual ~LifeQueryDialog();

    void query();
    void apply();
    void request(const std::string& url);
    void show_error(const Glib::ustring& msg, Gtk::MessageType type = Gtk::MessageType::MESSAGE_ERROR);
    void notify(Glib::Error& error, Glib::RefPtr<Glib::Bytes>& bytes);
    void content_changed();
    static void showDialog(
                  Gtk::Application *appl
                , ResultCallback* resultCallback
                , const  std::shared_ptr<LifeGrid>& lifeGrid);
protected:
    void view(LifeFileType fileType);
private:
    ResultCallback* m_resultCallback;
    Gtk::Button* m_query;
    Gtk::Button* m_apply;
    Gtk::Button* m_viewLife;
    Gtk::Button* m_viewRle;
    Gtk::Entry* m_src;
    Gtk::TextView* m_content;
    SoupSession* m_session{};
    Glib::RefPtr<Gio::Cancellable> m_cancellable;
    bool m_active{};
    std::shared_ptr<LifeGrid> m_lifeGrid;
};
