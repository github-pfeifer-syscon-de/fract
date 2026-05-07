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

#include "LifeParser.hpp"
#include "LifeDialog.hpp"


LifeDialog::LifeDialog(BaseObjectType* cobject
                    , const Glib::RefPtr<Gtk::Builder>& builder
                    , Gtk::Application *appl)
: Gtk::Dialog(cobject)
, m_appl{appl}
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
    builder->get_widget("open", m_open);
    builder->get_widget("rule",m_rule);

    m_apply->signal_clicked().connect(
        sigc::mem_fun(*this, &LifeDialog::apply));
    m_random->signal_clicked().connect(
        sigc::mem_fun(*this, &LifeDialog::random));
    m_clear->signal_clicked().connect(
        sigc::mem_fun(*this, &LifeDialog::clear));
    m_open->signal_clicked().connect(
        sigc::mem_fun(*this, &LifeDialog::open));
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

void
LifeDialog::updateMouse(double mx, double my, guint button)
{
    createGrid();
    auto ix = static_cast<int32_t>(mx / static_cast<double>(m_lifeGrid->getScaleFactor()));
    auto iy = static_cast<int32_t>(my / static_cast<double>(m_lifeGrid->getScaleFactor()));
    if (ix >= 0 && ix < m_lifeGrid->getWidth()
     && iy >= 0 && iy < m_lifeGrid->getHeight()) {
        m_lifeGrid->setCell(ix, iy, 1, button == GDK_BUTTON_PRIMARY);
        m_lifeGrid->update(m_imageSurface, m_colorRender->get_active());
        m_drawing->queue_draw();
    }
}

bool
LifeDialog::drawing_clicked(GdkEventButton* event)
{
    //std::cout << "x " << event->x << " y " << event->y << " btn " << event->button << std::endl;
    m_mouseButton = event->button;
    if (m_mouseButton != 0) {
        updateMouse(event->x, event->y, m_mouseButton);
    }
    return true;    // event handled
}

bool
LifeDialog::drawing_motion(GdkEventMotion* event)
{
    if (m_mouseButton != 0) {
        updateMouse(event->x, event->y, m_mouseButton);
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
    auto ruleName = m_rule->get_text();
    auto gridRule = m_lifeGrid->getRule();
    if (!ruleName.empty()
     && ruleName != gridRule->getName()) {
        auto dynRule = std::make_shared<DynamicLifeRule>(ruleName);
        auto ruleMsg = dynRule->getMessage();
        if (!ruleMsg.empty()) {
            show_error(ruleMsg);
            return;
        }
        m_lifeGrid->setRule(dynRule);
    }
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

std::string
LifeDialog::getDefaultSrc()
{
    return EXAMPLE_ADDRESS;
}

void
LifeDialog::notify(const std::string& content)
{
    if (m_timer) {
        m_timer.disconnect();   // stop update
    }
    createGrid();
    auto parser = LifeParser::getParser(content);
    if (!parser) {
        show_error("Format not recognized", Gtk::MessageType::MESSAGE_ERROR);
        return;
    }
    parser->setMinWidthHeight(
              static_cast<int32_t>(m_width->get_adjustment()->get_lower())
            , static_cast<int32_t>(m_height->get_adjustment()->get_lower()));
    Glib::ustring msg;
    m_lifeGrid = parser->parse(content, msg);
    if (!m_lifeGrid) {
        msg += "No result when parsing\n";
    }
    if (!msg.empty()) {
        show_error(msg, Gtk::MessageType::MESSAGE_WARNING);
    }
    if (m_lifeGrid) {
        m_rule->set_text(m_lifeGrid->getRule()->getName());
        m_width->set_value(m_lifeGrid->getWidth());
        m_height->set_value(m_lifeGrid->getHeight());
        adjustImageSize();
        m_lifeGrid->update(m_imageSurface, m_colorRender->get_active());
        m_drawing->queue_draw();
    }
}

void
LifeDialog::show_error(const Glib::ustring& msg, Gtk::MessageType type)
{
    // this shoud automatically give some context
    if (type == Gtk::MessageType::MESSAGE_ERROR) {
        g_warning("show_error %s", msg.c_str());
    }
    Gtk::MessageDialog messagedialog(*this, msg, FALSE, type);
    messagedialog.run();
    messagedialog.hide();
}

void
LifeDialog::open()
{
    LifeQueryDialog::showDialog(m_appl, this, m_lifeGrid);
}

void
LifeDialog::adjustImageSize()
{
    auto scaleFactor = m_lifeGrid->getScaleFactor();
    auto reqWidth{m_lifeGrid->getWidth() * scaleFactor};
    auto reqHeight{m_lifeGrid->getHeight() * scaleFactor};
    m_imageSurface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, reqWidth, reqHeight);
    m_drawing->set_size_request(reqWidth, reqHeight);
    set_size_request(reqWidth + 270, reqHeight + 60);   // scale dialog to view (found no working alternative)
}

void
LifeDialog::createGrid()
{
    int32_t width = m_width->get_value_as_int();
    int32_t height = m_height->get_value_as_int();
    if (!m_lifeGrid
      || m_lifeGrid->getWidth() != width
      || m_lifeGrid->getHeight() != height) {
        m_lifeGrid = std::make_shared<LifeGrid>(width, height);
        adjustImageSize();
    }
}

void
LifeDialog::showDialog(Gtk::Application *appl)
{
    auto refBuilder = Gtk::Builder::create();
    try {
        refBuilder->add_from_resource(appl->get_resource_base_path() + "/life-dlg.ui");
        LifeDialog* lifeDialog{};
        refBuilder->get_widget_derived("LifeDialog", lifeDialog, appl);
        lifeDialog->run();
        delete lifeDialog;  // delete top level
    }
    catch (const Glib::Error& ex) {
        std::cerr << "LifeDialog::showDialog: " << ex.what() << std::endl;
    }
}