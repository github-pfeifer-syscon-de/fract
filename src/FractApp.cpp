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

#include <gtkmm.h>
#include <iostream>
#include <exception>

#include "FractApp.h"

FractApp::FractApp(int argc, char **argv)
: Gtk::Application(argc, argv, "de.pfeifer_syscon.fract")
, m_fractAppWindow{}
{
}

FractApp::~FractApp()
{
}

void
FractApp::on_activate()
{
    m_fractAppWindow = new FractWin(std::make_shared<Param>(), this);
	add_window(*m_fractAppWindow);
	m_fractAppWindow->present();
}

void
FractApp::on_action_quit()
{
	m_fractAppWindow->hide();

	// Not really necessary, when Gtk::Widget::hide() is called, unless
	// Gio::Application::hold() has been called without a corresponding call
	// to Gio::Application::release().
	quit();
}

void
FractApp::on_startup()
{
	// Call the base class's implementation.
	Gtk::Application::on_startup();

	// Add actions and keyboard accelerators for the application menu.
	//add_action("preferences", sigc::mem_fun(m_monglAppWindow, &MonglAppWindow::on_action_preferences));
	//add_action("about", sigc::mem_fun(m_monglAppWindow, &MonglAppWindow::on_action_about));
	//add_action("quit", sigc::mem_fun(*this, &MonglApp::on_action_quit));
	//set_accel_for_action("app.quit", "<Ctrl>Q");

	//auto refBuilder = Gtk::Builder::create();
	//try {
	// /de/pfeifer_syscon/monglapp
	//	auto path = get_resource_base_path();
	//    refBuilder->add_from_resource(path + "/app-menu.ui");
	//    auto object = refBuilder->get_object("appmenu");
	//    auto app_menu = Glib::RefPtr<Gio::MenuModel>::cast_dynamic(object);
	//    if (app_menu)
	//        set_app_menu(app_menu);
	//    else
	//        std::cerr << "MonglApp::on_startup(): No \"appmenu\" object in app_menu.ui"
	//            << std::endl;
	//} catch (const Glib::Error& ex) {
	//    std::cerr << "MonglApp::on_startup(): " << ex.what() << std::endl;
	//    return;
	//}
}

int
main(int argc, char** argv)
{
	//printf("DISPLAY=%s\n", getenv("DISPLAY"));
	//setenv("DISPLAY", ":0", 1);    // Debug on remote display

	FractApp app(argc, argv);

	return app.run();
}