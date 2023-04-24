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

#include <gtkmm.h>
#include <dirent.h>
#include <stdlib.h>


#include "FractWin.h"

FractWin::FractWin(std::shared_ptr<Param> param, Gtk::Application *appl)
: Gtk::ApplicationWindow()
, m_fractView(nullptr)
{

	set_default_size(param->getWidth(), param->getHeight());
	auto scrWin = Gtk::make_managed<Gtk::ScrolledWindow>();
	m_fractView = Gtk::make_managed<FractView>(*this, param, appl);
	Glib::RefPtr<Gdk::Pixbuf> pix = Gdk::Pixbuf::create_from_resource(appl->get_resource_base_path() + "/fract.png");
	set_icon(pix);

	scrWin->add(*m_fractView);
	add(*scrWin);

	show_all_children();
}

FractWin::~FractWin()
{
}

void
FractWin::on_hide()
{
	Gtk::Window::on_hide();
	// stop still running worker, as it will crash
	//   otherwise on accessing a destoryed pixmap
	if (m_fractView != nullptr)
		m_fractView->stop_workers(false);
}

FractView *
FractWin::getFractView()
{
	return m_fractView;
}
