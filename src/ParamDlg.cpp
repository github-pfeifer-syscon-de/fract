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


#include "ParamDlg.h"

ParamDlg::ParamDlg(Gtk::Window &parent, std::shared_ptr<Param> param)
: m_color_cbs()
{
	auto grid = Gtk::make_managed<Gtk::Grid>();
	set_transient_for(parent);
	set_title("Parameter");

	m_Samples.append("1");
	m_Samples.append("2");
	//m_Samples.append("3");
	m_Samples.append("4");

	m_Function.append(code2name('M'));
	m_Function.append(code2name('J'));
	m_Function.append(code2name('N'));

	auto lblSamples = Gtk::make_managed<Gtk::Label>("Samples ");
	grid->attach(*lblSamples, 0, 0, 1, 1);
	grid->attach(m_Samples, 1, 0, 1, 1);

	auto lblWidth = Gtk::make_managed<Gtk::Label>("Width ");
	grid->attach(*lblWidth, 0, 1, 1, 1);
	grid->attach(m_Width, 1, 1, 1, 1);
	m_Width.set_range(64, 4096);
	m_Width.set_increments(16, 256);

	auto lblHeight = Gtk::make_managed<Gtk::Label>("Height ");
	grid->attach(*lblHeight, 0, 2, 1, 1);
	grid->attach(m_Height, 1, 2, 1, 1);
	m_Height.set_range(64, 4096);
	m_Height.set_increments(16, 256);

	auto lblDepth = Gtk::make_managed<Gtk::Label>("Depth ");
	grid->attach(*lblDepth, 0, 3, 1, 1);
	grid->attach(m_Depth, 1, 3, 1, 1);
	m_Depth.set_range(16, 8192);
	m_Depth.set_increments(16, 256);

	auto lblFunction = Gtk::make_managed<Gtk::Label>("Function ");
	grid->attach(*lblFunction, 0, 4, 1, 1);
	grid->attach(m_Function, 1, 4, 1, 1);

	for (gint32 i = 0; i < param->get_color_count(); ++i) {
		const auto lblText = Glib::ustring::sprintf("Color %d", i);
		auto lblFirst = Gtk::make_managed<Gtk::Label>(lblText);
		grid->attach(*lblFirst, 0, 5 + i, 1, 1);
		auto cbs = Gtk::make_managed<Gtk::ColorButton>();
		cbs->set_color(param->get_color(i));
		m_color_cbs.push_back(cbs);
		grid->attach(*cbs, 1, 5 + i, 1, 1);
	}

	get_vbox()->pack_start(*grid);

	m_Samples.set_active_text(Glib::ustring::sprintf("%d", param->getSamples()));

	m_Depth.set_value(param->getDepth());

	m_Width.set_value(param->getWidth());

	m_Height.set_value(param->getHeight());

	m_Function.set_active_text(code2name(param->getFunction()));

	auto button_Close = Gtk::make_managed<Gtk::Button>("Ok");
	add_action_widget(*button_Close, Gtk::RESPONSE_OK);

	//m_Depth.grab_default();
	show_all_children();
}

Glib::ustring
ParamDlg::code2name(char code)
{
	switch (code) {
	case 'M':
	case 'm':
		return Glib::ustring("Mandelbrot");
	case 'J':
	case 'j':
		return Glib::ustring("Juliaset");
	case 'N':
	case 'n':
		return Glib::ustring("Newton");
	default:
		char tmp[16];
		snprintf(tmp, sizeof(tmp), "%c", code);
		return Glib::ustring(tmp);
	}
}

ParamDlg::~ParamDlg()
{
}

void
ParamDlg::refresh(std::shared_ptr<Param> param)
{
	param->setSamples(atoi(m_Samples.get_active_text().data()));
	param->setDepth(m_Depth.get_value_as_int());
	param->setHeight(m_Height.get_value_as_int());
	param->setWidth(m_Width.get_value_as_int());
	param->setFunction(m_Function.get_active_text().data()[0]);

	std::vector<Gtk::ColorButton *>::iterator cbsi = m_color_cbs.begin();
	for (gint32 i = 0; i < param->get_color_count(); ++i, ++cbsi) {
		auto cbs = *cbsi;
		param->set_color(i, cbs->get_color());
	}
}
