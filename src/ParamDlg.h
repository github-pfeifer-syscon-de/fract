/*
 * Copyright (C) 2018 rpf
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

#include "Param.h"

class ParamDlg : public Gtk::Dialog {
public:
    ParamDlg(Gtk::Window &parent, std::shared_ptr<Param> param);
    virtual ~ParamDlg();

    void refresh(std::shared_ptr<Param> param);
protected:
private:
    Gtk::ComboBoxText m_Samples;
    Gtk::SpinButton m_Depth;
    Gtk::ComboBoxText m_Function;
    Gtk::SpinButton m_Width;
    Gtk::SpinButton m_Height;
    std::vector<Gtk::ColorButton *> m_color_cbs;
    Glib::ustring code2name(char code);
};
