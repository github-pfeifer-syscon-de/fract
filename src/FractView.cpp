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


#include <cairo/cairo.h>
#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <exception>

#include "FractView.h"
#include "ParamDlg.h"
#include "FractWin.h"

FractView::FractView(Gtk::Window &_parent, std::shared_ptr<Param> _param, Gtk::Application *appl)
: m_param{_param}
, m_Dispatcher()
, m_notifyMutex()
, m_redrawMutex()
, m_selectedRect{nullptr}
, m_parent{_parent}
, m_appl{appl}
, m_worker{nullptr}
, m_proc{g_get_num_processors()} // uses number of cores, and stick to it, posix : sysconf(_SC_NPROCESSORS_ONLN) gtk is more portable (and gives the same answer)
, m_row{0}
, m_lastParam()
{

	add_events(Gdk::EventMask::BUTTON_PRESS_MASK
		| Gdk::EventMask::BUTTON_RELEASE_MASK
		| Gdk::EventMask::BUTTON_MOTION_MASK);
	m_Dispatcher.connect(sigc::mem_fun(*this, &FractView::on_notification_from_worker_thread));

	start_new();
}

FractView::~FractView()
{
	// we have done this on hide already this should not be required,
	//   but just to be safe as if any thread remains we will crash
	stop_workers(true);
	// m_lastParam and m_param is managed internally
	if (m_selectedRect != nullptr)
		delete m_selectedRect;
	m_selectedRect = nullptr;
	if (m_worker != nullptr)
		delete m_worker;
	m_worker = nullptr;
}

bool
FractView::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	if (!m_pixmap) {
		return true;
	}
	//cr-> = event->region;
	//Gtk::Allocation allocation = get_allocation();
    {
        // as we had some issues when filling imageSurface asynchronously
        // use a lock to serialize update/drawing
        std::lock_guard<std::mutex> lock(m_redrawMutex);
        //cr->rectangle(allocation.get_x(), allocation.get_y(),
        //    allocation.get_width(), allocation.get_height());
        cr->set_antialias(Cairo::Antialias::ANTIALIAS_SUBPIXEL);
        cr->save();
        cr->set_source_rgb(0.0, 0.0, 0.0);
        cr->scale(1.0 / (double) m_param->getSamples(), 1.0 / (double) m_param->getSamples());
        cr->set_source(m_pixmap, 0, 0);
        cr->paint();
    }

    if (m_selectedRect) {
        cr->restore();
        cr->set_line_width(1.0);
        cr->set_source_rgb(1.0, 1.0, 1.0);
        cr->rectangle(m_selectedRect->x, m_selectedRect->y,
            m_selectedRect->width, m_selectedRect->height);
        cr->stroke();
    }
	//std::cout << "End paint\n";
	return true;
}

bool
FractView::on_motion_notify_event(GdkEventMotion* event)
{
	//std::cout << "mot pos " << event->x << "," << event->y
	//          << " state " <<  event->state
	//          << " type " << event->type << "\n";

	if (m_selectedRect) {
		Cairo::RectangleInt lastRect = *m_selectedRect;

		if (event->x < m_selectedRect->x)
			m_selectedRect->x = event->x;
		else
			m_selectedRect->width = event->x - m_selectedRect->x;

		if (event->y < m_selectedRect->y)
			m_selectedRect->y = event->y;
		else
			m_selectedRect->height = event->y - m_selectedRect->y;

		Cairo::RectangleInt r = *m_selectedRect;
		int xm = std::max(r.x + r.width, lastRect.x + lastRect.width);
		int ym = std::max(r.y + r.height, lastRect.y + lastRect.height);
		r.x = std::min(r.x, lastRect.x);
		r.y = std::min(r.y, lastRect.y);
		r.width = xm - r.x;
		r.height = ym - r.y;

		//std::cout << "mov " << selectedRect->x << "," << selectedRect->y
		//      << " w " <<  selectedRect->width << "," << selectedRect->height << endl;
		Gtk::Allocation allocation = get_allocation();
		queue_draw_area(r.x + allocation.get_x(),
			r.y + allocation.get_y(),
			r.width + 1, r.height + 1);
	}

	return true;
}

bool
FractView::on_button_release_event(GdkEventButton* event)
{
	//std::cout << "rel btn " << event->button
	//          << " state " <<  event->state
	//         << " type " << event->type << "\n";
	if (m_selectedRect) {
		//std::cout << "rel " << selectedRect->x << "," << selectedRect->y
		//      << " w " <<  selectedRect->width << "," << selectedRect->height << endl;
		Cairo::RectangleInt saveRect = *m_selectedRect;
		delete m_selectedRect;
		m_selectedRect = nullptr;
		Gtk::Allocation allocation = get_allocation();
		queue_draw_area(saveRect.x + allocation.get_x(),
			saveRect.y + allocation.get_y(),
			saveRect.width + 1, saveRect.height + 1);

		float ratio = (float) m_param->getWidth() / (float) m_param->getHeight();
		// Keep the ratio
		saveRect.height = (int) ((float) saveRect.width / ratio);

		std::shared_ptr<Param> next = m_param->update(saveRect);
		m_lastParam.push_back(m_param);
		m_param = next;
		start_new();
	}
	return true;
}

void
FractView::start_new()
{
	// param shoud be set before
	// use this to debug
	//m_param->info(cout);
	m_row = 0;
	reinit_redraw();
	fill();
}

bool
FractView::on_button_press_event(GdkEventButton* event)
{
	//std::cout << "press btn " << event->button << endl;
	//          << " state " <<  event->state
	//          << " type " << event->type << "\n";
	if (event->button == 1) {
		if (m_selectedRect == nullptr) {
			m_selectedRect = new(Cairo::RectangleInt);
			m_selectedRect->x = event->x;
			m_selectedRect->y = event->y;
			m_selectedRect->width = 1;
			m_selectedRect->height = 1;
		}
		//std::cout << "pres " << selectedRect->x << "," << selectedRect->y
		//          << " w " <<  selectedRect->width << "," << selectedRect->height << endl;
		Gtk::Allocation allocation = get_allocation();
		queue_draw_area(m_selectedRect->x + allocation.get_x(),
			m_selectedRect->y + allocation.get_y(),
			m_selectedRect->width, m_selectedRect->height);

		return true; // It has been handled.
	}
	else if (event->button == 3) {
		if (m_param->isPrimaryWindow()) {
			// Dont change julia points as we dont allow sub, sub windows see below and want to keep actual view
			m_param->setReJulia(m_param->getReStart() + m_param->getReStep() * event->x);
			m_param->setImJulia(m_param->getImStart() + m_param->getImStep() * event->y);
		}

		Gtk::Menu* popupMenu = build_popup();
		// deactivate prevent item signals to get generated ...
		// signal_unrealize will never get generated
		popupMenu->attach_to_widget(*this); // this does the trick and calls the destructor
		popupMenu->popup(event->button, event->time);

		return true; // It has been handled.
	}
	return false;
}

Gtk::Menu *
FractView::build_popup()
{
	// managed works when used with attach ...
	auto pMenuPopup = Gtk::make_managed<Gtk::Menu>();
	auto about = Gtk::make_managed<Gtk::MenuItem>("_About", true);
	about->signal_activate().connect(sigc::mem_fun(*this, &FractView::on_action_about));
	pMenuPopup->append(*about);
	auto save = Gtk::make_managed<Gtk::MenuItem>("_Save", true);
	save->signal_activate().connect(sigc::mem_fun(*this, &FractView::on_menu_save));
	pMenuPopup->append(*save);
	auto open = Gtk::make_managed<Gtk::MenuItem>("_Open", true);
	open->signal_activate().connect(sigc::mem_fun(*this, &FractView::on_menu_open));
	pMenuPopup->append(*open);
	auto mparam = Gtk::make_managed<Gtk::MenuItem>("_Parameter", true);
	mparam->signal_activate().connect(sigc::mem_fun(*this, &FractView::on_menu_param));
	pMenuPopup->append(*mparam);
	if (m_param->isPrimaryWindow()) {
		auto julia = Gtk::make_managed<Gtk::MenuItem>("_Julia", true);
		julia->signal_activate().connect(sigc::mem_fun(*this, &FractView::on_menu_julia));
		pMenuPopup->append(*julia);
	}
	if (m_lastParam.size() > 0) {
		auto last = Gtk::make_managed<Gtk::MenuItem>("Last", true);
		pMenuPopup->append(*last);
		auto subMenu = Gtk::make_managed<Gtk::Menu>();
		last->set_submenu(*subMenu);
		guint n = 0;
		std::list<std::shared_ptr < Param>>::iterator p;
		for (p = m_lastParam.begin(); p != m_lastParam.end(); p++) {
			Glib::ustring name;
			if (n == 0) {
				name = "first";
			}
			else if (n == m_lastParam.size() - 1) {
				name = "recent";
			}
			else {
				name = Glib::ustring::sprintf("%d", n);
			}
			auto lastN = Gtk::make_managed<Gtk::MenuItem>(name, true);
			lastN->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &FractView::on_menu_last), n));
			subMenu->append(*lastN);
			++n;
		}
	}

	pMenuPopup->show_all();
	return pMenuPopup;
}

void
FractView::on_menu_save()
{
	Gtk::FileChooserDialog dialog("Save file", Gtk::FILE_CHOOSER_ACTION_SAVE);
	dialog.set_transient_for(m_parent);

	//Add response buttons the the dialog:
	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("_Save", Gtk::RESPONSE_OK);

	//Add filters, so that only certain file types can be selected:

	auto filter_image_png = Gtk::FileFilter::create();
	filter_image_png->set_name("Png files");
	filter_image_png->add_mime_type("image/png");
	dialog.add_filter(filter_image_png);
	int result = dialog.run();
	switch (result) {
	case Gtk::RESPONSE_OK:
		save_png(dialog.get_filename());
		break;
	}
}

void
FractView::save_png(Glib::ustring filename)
{
	std::cout << "on save " << filename << std::endl;
	Cairo::RefPtr<Cairo::ImageSurface> outpixmap = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
		m_param->getWidth(),
		m_param->getHeight());

	{
		Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(outpixmap);
		cr->set_antialias(Cairo::Antialias::ANTIALIAS_SUBPIXEL);
		cr->set_source_rgb(0.0, 0.0, 0.0);
		cr->scale(1.0 / (double) m_param->getSamples(), 1.0 / (double) m_param->getSamples());
		cr->set_source(m_pixmap, 0, 0);
		cr->paint();
	}
	m_param->save(filename + ".txt");

	outpixmap->write_to_png(filename);
}

void
FractView::on_menu_param()
{
	ParamDlg paramDlg(m_parent, m_param);
	if (paramDlg.run() == Gtk::RESPONSE_OK) {
		paramDlg.refresh(m_param);
		start_new();
	}
}

std::shared_ptr<Param>
FractView::get_param()
{
	return m_param;
}

Cairo::RefPtr<Cairo::ImageSurface>
FractView::get_imagesurface()
{
	return m_pixmap;
}

void
FractView::on_menu_last(guint n)
{
	// truncate list as we do not handle branches
	guint i = 0;
	std::list<std::shared_ptr<Param> >::iterator p;
	for (p = m_lastParam.begin(); p != m_lastParam.end();) {
		if (i > n) {
			std::shared_ptr<Param> param = *p;
			p = m_lastParam.erase(p);
		}
		else {
			p++;
		}
		++i;
	}
	// one param shoud always remain
	p = m_lastParam.end();
	--p;
	m_param = *p;
	m_lastParam.erase(p); // remove also from list to prevent double free (and it will get added again if we zoom)
	start_new();

}

void
FractView::on_menu_julia()
{
	auto sparam = std::make_shared<Param>(*m_param);
	sparam->setFunction('J');
	sparam->setReStart(-2.0); // start from overview
	sparam->setReEnd(2.0);
	sparam->setImStart(-1.5);
	sparam->setImEnd(1.5);
	sparam->setPrimaryWindow(false); // to keep the effort for memory management to a reasonable amount don't allow creating sub, sub windows

	FractWin *fract_window = new FractWin(sparam, m_appl);
	//m_ChildFract.push_back(fract_window);
	fract_window->signal_hide().connect(
		sigc::bind<gpointer>(
		sigc::mem_fun(*this, &FractView::fractWinClose),
		(gpointer) fract_window));
	fract_window->show();
}

void
FractView::fractWinClose(gpointer fract)
{
	if (fract) {
		//m_ChildFract.remove(fract);
		delete (FractWin *) fract;
	}
}

void
FractView::fill()
{
	// make previous threads exit as we dont handle concurrent updates
	stop_workers(true);

	m_param->build_color_map();
	set_size_request(m_param->getWidth(), m_param->getHeight());
	if (!m_pixmap ||
		m_pixmap->get_width() != (gint)m_param->getPixmapWidth() ||
		m_pixmap->get_height() != (gint)m_param->getPixmapHeight()) {
		m_pixmap = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
			m_param->getPixmapWidth(),
			m_param->getPixmapHeight());

		Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_pixmap);
		/* Erase pixmap */
		cr->set_source_rgb(0.0, 0.0, 0.0);
		cr->rectangle(0, 0, m_pixmap->get_width(), m_pixmap->get_height());
		cr->paint();
	}

	if (m_worker == nullptr) {
		m_worker = new Worker<double>*[m_proc];
	}
	for (guint32 i = 0; i < m_proc; ++i) {
		m_worker[i] = nullptr;
		switch (m_param->getFunction()) {
		case 'M':
			m_worker[i] = new MandelWorker<double>(this);
			break;
		case 'J':
			m_worker[i] = new JuliaWorker<double>(this);
			break;
		case 'N':
			m_worker[i] = new NewtonWorker<double>(this);
			break;
		}
		if (m_worker[i]) {
			// http://www.acodersjourney.com/2017/08/top-20-cplusplus-multithreading-mistakes/
			//std::future<void> asyncWorker = std::async(std::launch::async, [t_work, i]
			//std::cout << "Started worker " << i << "." << std::endl;
			std::thread workerThread([this, i]
			{
				try {
					m_worker[i]->do_work();
				} catch (std::exception &e) {
					std::cerr << "Exception on worker " << i
						<< " " << e.what()
						<< std::endl;
				}
				// Cleanup in any case
				delete m_worker[i];
				m_worker[i] = nullptr;
				//std::cout << "Exited worker " << i << "." << std::endl;
			});
			workerThread.detach(); // make it a background thread, necessary for not to crash on free
		}
	}

}

guint32
FractView::get_row()
{
	//std::lock_guard<std::mutex> lock(m_Row);
	return m_row++;
}

void
FractView::reinit_redraw()
{
	m_redraw_start = m_param->getHeight();
	m_redraw_end = 0;
	m_redraw_pending = false;
}

void
FractView::notifyRow(guint row, unsigned int *image)
{
	std::lock_guard<std::mutex> lock(m_notifyMutex);
    // try to modify in a locked context
	guint stride = m_pixmap->get_stride(); // related to image format ARGB32 index byte to guint32 as we use a pointer of that type
	auto img = m_pixmap->get_data();
	int i = (row * stride); // related to image format ARGB32, matches pointer guint32
    {
        std::lock_guard<std::mutex> lock(m_redrawMutex);
        m_pixmap->mark_dirty(0, row, m_param->getWidth(), 1);
        memcpy(&img[i], image, m_pixmap->get_stride());
    }
	guint srow = row / m_param->getSamples();
	if (m_redraw_start > srow)
		m_redraw_start = srow;
	if (m_redraw_end < srow)
		m_redraw_end = srow;
	//std::cout << "Added row " << row << " area " << redraw_start << " end " << redraw_end << "\n";

	if (!m_redraw_pending) {
		m_redraw_pending = true; // as dispatching takes some time do no bother again
		m_Dispatcher.emit(); // break out of thread box
	}
}

void
FractView::on_notification_from_worker_thread()
{
	gint32 redraw_row;
	gint32 redraw_height;
	{
		std::lock_guard<std::mutex> lock(m_notifyMutex); // block concurrent updates
		redraw_row = m_redraw_start; // just for a short moment to read the area
		redraw_height = m_redraw_end - m_redraw_start + 1;
		reinit_redraw();
	}
	//std::cout << "Draw area " << redraw_row << " " << redraw_height << "\n";
	//m_pixmap->mark_dirty(0, redraw_row, m_param->getWidth(), redraw_height);

	Gtk::Allocation allocation = get_allocation();
	if (redraw_row + redraw_height == allocation.get_height()) {
		queue_draw(); // as a last step redraw all as for some l&f we may have missed some
	}
	else {
		queue_draw_area(allocation.get_x(), redraw_row + allocation.get_y(), m_param->getWidth(), redraw_height);
	}
}

void
FractView::stop_workers(bool waitComplete)
{
	if (m_worker != nullptr) {
		for (guint32 i = 0; i < m_proc; ++i) {
			Worker<double> *wrk = m_worker[i];
			if (wrk != nullptr) {
				//std::cout << "Stopping workers " << i << "." << std::endl;
				wrk->setActive(false); // for an early exit prevent updating a vanished pixmap
				if (waitComplete) {
					int n = 0;
					while (m_worker[i] != nullptr
						&& n < 100) {
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						++n;
					}
				}
			}
		}
	}
	// no need to recurse as we wired this to every window instance
}

void
FractView::on_action_about()
{
	auto refBuilder = Gtk::Builder::create();
	try {
		refBuilder->add_from_resource(m_appl->get_resource_base_path() + "/abt-dlg.ui");
		auto object = refBuilder->get_object("abt-dlg");
		auto abtdlg = Glib::RefPtr<Gtk::AboutDialog>::cast_dynamic(object);
		if (abtdlg) {
			Glib::RefPtr<Gdk::Pixbuf> pix = Gdk::Pixbuf::create_from_resource(m_appl->get_resource_base_path() + "/fract.png");
			abtdlg->set_logo(pix);
			abtdlg->set_transient_for(m_parent);
			abtdlg->run();
			abtdlg->hide();
		}
		else
			std::cerr << "FractView::on_action_about(): No \"abt-dlg\" object in abt-dlg.ui"
			<< std::endl;
	} catch (const Glib::Error& ex) {
		std::cerr << "FractView::on_action_about(): " << ex.what() << std::endl;
	}
}

void
FractView::on_menu_open()
{
	Gtk::FileChooserDialog dialog("Open file", Gtk::FILE_CHOOSER_ACTION_SAVE);
	dialog.set_transient_for(m_parent);

	//Add response buttons the the dialog:
	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("_Open", Gtk::RESPONSE_OK);

	//Add filters, so that only certain file types can be selected:

	auto filter_image_png = Gtk::FileFilter::create();
	filter_image_png->set_name("Text files");
	filter_image_png->add_mime_type("text/plain");
	dialog.add_filter(filter_image_png);
	int result = dialog.run();
	switch (result) {
	case Gtk::RESPONSE_OK:
		std::shared_ptr<Param> next = m_param->open(dialog.get_filename());
		m_lastParam.push_back(m_param);
		m_param = next;
		start_new();
		break;
	}
}


