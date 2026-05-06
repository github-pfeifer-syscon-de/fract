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

#include "LifeGrid.hpp"
#include "LifeQueryDialog.hpp"

LifeQueryDialog::LifeQueryDialog(BaseObjectType* cobject
                    , const Glib::RefPtr<Gtk::Builder>& builder
                    , ResultCallback* resultCallback
                    , const std::shared_ptr<LifeGrid>& lifeGrid)
: Gtk::Dialog(cobject)
, m_resultCallback{resultCallback}
, m_lifeGrid{lifeGrid}
{
    builder->get_widget("buttonQuery", m_query);
    builder->get_widget("buttonApply", m_apply);
    builder->get_widget("src", m_src);
    builder->get_widget("content", m_content);
    builder->get_widget("viewLife", m_viewLife);
    builder->get_widget("viewRle", m_viewRle);

    m_viewLife->set_sensitive(m_lifeGrid.operator bool());
    m_viewRle->set_sensitive(m_lifeGrid.operator bool());
    m_src->set_text(m_resultCallback->getDefaultSrc());
    m_query->signal_clicked().connect(
        sigc::mem_fun(*this, &LifeQueryDialog::query));
    m_apply->signal_clicked().connect(
        sigc::mem_fun(*this, &LifeQueryDialog::apply));
    m_content->get_buffer()->signal_changed().connect(
        sigc::mem_fun(*this, &LifeQueryDialog::content_changed));
    m_viewRle->signal_clicked().connect(
        sigc::bind(
            sigc::mem_fun(*this, &LifeQueryDialog::view), LifeFileType::Rle));
    m_viewLife->signal_clicked().connect(
        sigc::bind(
            sigc::mem_fun(*this, &LifeQueryDialog::view), LifeFileType::Life105));
    content_changed();
}

LifeQueryDialog::~LifeQueryDialog()
{
    if (m_active && m_cancellable) {
        m_cancellable->cancel();
    }
    if (m_session) {
        g_object_unref(m_session);
        m_session = nullptr;
    }
}

void
LifeQueryDialog::query()
{
    std::string url = m_src->get_text();
    request(url);
}

void
LifeQueryDialog::apply()
{
    m_resultCallback->notify(m_content->get_buffer()->get_text());
    // has response id -> dialog will close
}

static void
on_load_callback(GObject *source, GAsyncResult *result, gpointer user_data)
{
    GError *error{};
    GBytes *bytes = soup_session_send_and_read_finish(SOUP_SESSION (source), result, &error);
    std::cout << "on_load_callback err " << error << " bytes " << bytes << std::endl;
    auto err = Glib::Error(error, false);    // "consume" pointer
    auto byte = Glib::wrap(bytes, false);
    auto lifeDialog = reinterpret_cast<LifeQueryDialog*>(user_data);
    lifeDialog->notify(err, byte);
    // Usage here is the same as before
    //if (error) {
    //    g_error_free (error);
    //} else {
    //    g_bytes_unref (bytes);
    //}
}

void
LifeQueryDialog::request(const std::string& url)
{
    if (!m_cancellable) {
        m_cancellable = Gio::Cancellable::create();
    }
    if (!m_session) {
        m_session = soup_session_new();
    }
    SoupMessage *msg = soup_message_new(SOUP_METHOD_GET, url.c_str());
    soup_session_send_and_read_async(
        m_session,
        msg,
        G_PRIORITY_DEFAULT,
        m_cancellable->gobj(),
        on_load_callback,
        this);
    m_active = true;
    g_object_unref(msg);
}

void
LifeQueryDialog::show_error(const Glib::ustring& msg, Gtk::MessageType type)
{
    // this shoud automatically give some context
    g_warning("show_error %s", msg.c_str());
    Gtk::MessageDialog messagedialog(*this, msg, FALSE, type);
    messagedialog.run();
    messagedialog.hide();
}

void
LifeQueryDialog::notify(Glib::Error& error, Glib::RefPtr<Glib::Bytes>& bytes)
{
    m_active = false;
    if (error) {
        show_error(Glib::ustring::sprintf("Error for request %s", error.what()));
    }
    else if (bytes) {
        size_t len;
        auto data = reinterpret_cast<const char*>(bytes->get_data(len));
        auto content = std::string(data, len);
        //std::cout << "Content " << content << std::endl;
        m_content->get_buffer()->set_text(content);
    }
    else {
        show_error("No content received");
    }
}

void
LifeQueryDialog::content_changed()
{
    auto text = m_content->get_buffer()->get_text();
    m_apply->set_sensitive(!text.empty());
}

void LifeQueryDialog::view(LifeFileType fileType)
{
    std::unique_ptr<LifeParser> lifeParser;
    switch (fileType) {
    case LifeFileType::Rle:
        lifeParser = std::make_unique<RleLifeParser>();
        break;
    default:
        lifeParser = std::make_unique<Life105LifeParser>();
        break;
    }
    if (m_lifeGrid) {
        std::string content = lifeParser->exportGrid(m_lifeGrid);
        m_content->get_buffer()->set_text(content);
    }
}

void
LifeQueryDialog::showDialog(
          Gtk::Application *appl
        , ResultCallback* resultCallback
        , const  std::shared_ptr<LifeGrid>& lifeGrid)
{
    auto refBuilder = Gtk::Builder::create();
    try {
        refBuilder->add_from_resource(appl->get_resource_base_path() + "/query-dlg.ui");
        LifeQueryDialog* queryDialog{};
        refBuilder->get_widget_derived("QueryDialog", queryDialog, resultCallback, lifeGrid);
        queryDialog->run();
        delete queryDialog;  // delete top level
    }
    catch (const Glib::Error& ex) {
        std::cerr << "QueryDialog::showDialog(): " << ex.what() << std::endl;
    }
}

