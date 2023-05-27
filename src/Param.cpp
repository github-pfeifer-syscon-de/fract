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


#include <fstream>
#include <iostream>
#include <algorithm>

#include "Param.h"

static const char* const RE_START = "reStart: ";
static const char* const RE_END = "reEnd: ";
static const char* const IM_START = "imStart: ";
static const char* const IM_END = "imEnd: ";
static const char* const DEPTH = "depth: ";
static const char* const FUNCTION = "function: ";
static const char* const WIDTH = "width: ";
static const char* const HEIGHT = "height: ";
static const char* const SAMPLES = "samples: ";
static const char* const COLORING = "coloring: ";
static const char* const RE_JULIA = "reJulia: ";
static const char* const IM_JULIA = "imJulia: ";

Param::Param()
: m_function('M')
, m_samples(1) // applied in x & y direction so value 4 resuts in 16 samples
, m_width(1024)
, m_height(768)
, m_depth(128)
, m_re_start(-2.0)
, m_re_end(0.5)
, m_im_start(-1.25)
, m_im_end(1.25)
, m_re_julia(0.23543446355295022)
, m_im_julia(-0.5198893148651992)
, m_color_map(nullptr)
, m_primaryWindow(true)
, m_colors()
, m_coloring(STRECH)
{
	// https://stackoverflow.com/questions/16500656/which-color-gradient-is-used-to-color-mandelbrot-in-wikipedia
	m_colors.push_back(Gdk::Color("#000764"));
	m_colors.push_back(Gdk::Color("#206bcb"));
	m_colors.push_back(Gdk::Color("#edffff"));
	m_colors.push_back(Gdk::Color("#ffaa00"));
	//m_colors.push_back(Gdk::Color("#000200"));    // as we dont use the gradient wrap
}

Param::Param(const Param& orig)
: m_function(orig.m_function)
, m_samples(orig.m_samples)
, m_width(orig.m_width)
, m_height(orig.m_height)
, m_depth(orig.m_depth)
, m_re_start(orig.m_re_start)
, m_re_end(orig.m_re_end)
, m_im_start(orig.m_im_start)
, m_im_end(orig.m_im_end)
, m_re_julia(orig.m_re_julia)
, m_im_julia(orig.m_im_julia)
, m_color_map(nullptr) // as this can easily be recreated and we dont want to share the pointer (freeing it two times is not so an good idea)
, m_primaryWindow(orig.m_primaryWindow)
, m_colors(orig.m_colors)
, m_coloring(orig.m_coloring)
{
}

Param::~Param()
{
	//std::cout << "destructor param" << std::endl;
	if (m_color_map)
		g_free(m_color_map);

}

void
Param::setDepth(guint depth)
{
	m_depth = depth;
}

guint
Param::getDepth()
{
	return m_depth;
}

void
Param::setWidth(guint width)
{
	m_width = width;
}

guint
Param::getWidth()
{
	return m_width;
}

void
Param::setHeight(guint height)
{
	m_height = height;
}

guint
Param::getHeight()
{
	return m_height;
}

guint
Param::getPixmapWidth()
{
	return m_width * m_samples;
}

guint
Param::getPixmapHeight()
{
	return m_height * m_samples;
}

void
Param::setFunction(gchar function)
{
	m_function = function;
}

gchar
Param::getFunction()
{
	return m_function;
}

void
Param::setReStart(long double reStart)
{
	m_re_start = reStart;
}

void
Param::setReEnd(long double reEnd)
{
	m_re_end = reEnd;
}

void
Param::setImStart(long double imStart)
{
	m_im_start = imStart;
}

void
Param::setImEnd(long double imEnd)
{
	m_im_end = imEnd;
}

long double
Param::getImStep()
{
	return(m_im_end - m_im_start) / (long double) (getPixmapHeight() - 1);
}

long double
Param::getReStep()
{
	return(m_re_end - m_re_start) / (long double) (getPixmapWidth() - 1);
}

long double
Param::getImStart()
{
	return m_im_start;
}

long double
Param::getReStart()
{
	return m_re_start;
}

long double
Param::getReJulia()
{
	return m_re_julia;
}

long double
Param::getImJulia()
{
	return m_im_julia;
}

void
Param::setReJulia(long double reJulia)
{
	m_re_julia = reJulia;
}

void
Param::setImJulia(long double imJulia)
{
	m_im_julia = imJulia;
}

bool
Param::isPrimaryWindow()
{
	return m_primaryWindow;
}

void
Param::setPrimaryWindow(bool primary)
{
	m_primaryWindow = primary;
}

void
Param::setSamples(guint32 samp)
{
	this->m_samples = samp;
}

guint32
Param::getSamples() const
{
	return m_samples;
}

std::shared_ptr<Param>
Param::update(Cairo::RectangleInt saveRect)
{
	saveRect.x *= m_samples; // convert screen to internal
	saveRect.y *= m_samples;
	saveRect.width *= m_samples;
	saveRect.height *= m_samples;

	long double im_step = getImStep();
	long double re_step = getReStep();
	auto ret = std::make_shared<Param>(*this);
	ret->m_re_start = m_re_start + re_step * (long double) saveRect.x;
	ret->m_re_end = ret->m_re_start + re_step * (long double) (saveRect.width);
	ret->m_im_start = m_im_start + im_step * (long double) saveRect.y;
	ret->m_im_end = ret->m_im_start + im_step * (long double) (saveRect.height);

	return ret;
}

void
Param::build_color_map()
{
	if (m_color_map)
		g_free(m_color_map);

	m_color_map = g_new0(guint32, m_depth);
	double depth_step = (double) m_depth / (double) m_colors.size();
	for (guint32 d = 0; d < m_depth; ++d) {
		if (m_coloring == STRECH) {
			m_color_map[d] = map(d, depth_step);
		}
	}
}

guint32
Param::map(gint32 d, double depth_step)
{
	guint32 i = (guint32) ((double) d / depth_step);
	guint32 lastValue = i * depth_step;
	guint32 iv = (i + 1) * depth_step;
	Gdk::Color color = m_colors[i];
	gint32 c = (d - lastValue) * 255 * m_colors.size() / m_depth;
	guint32 r = (guint32) (color.get_red_p() * c);
	guint32 g = (guint32) (color.get_green_p() * c);
	guint32 b = (guint32) (color.get_blue_p() * c);
	if (i > 0) {
		Gdk::Color lastColor = m_colors[i - 1];
		gint32 c = (iv - d) * 255 * m_colors.size() / m_depth;
		r += (guint32) (lastColor.get_red_p() * c);
		g += (guint32) (lastColor.get_green_p() * c);
		b += (guint32) (lastColor.get_blue_p() * c);
	}

	//std::cout << "d: " << d << " r 0x" << std::hex << r << " g 0x" << g << " b 0x"  << b << std::dec << std::endl;
	r = std::min(std::max(r, 0u), 0xffu);
	g = std::min(std::max(g, 0u), 0xffu);
	b = std::min(std::max(b, 0u), 0xffu);
	//       A 31..24       R 23..16         G 15..8         B 7..0
	return 0xff000000u | ((r) << 0x10) | ((g) << 0x08) | (b);
}

guint32
Param::map_rgb(guint32 d, long double re, long double im)
{
	if (d < 0 || d >= m_depth) // as we actual reach the depth handle these well
		return 0xff000000u; // for black, opaque
	//double smoothed = log2(log2(re * re + im * im) / 2.0);  // log_2(log_2(|p|))
	//int colorI = (int)(sqrt(d + 10.0 - smoothed) * 256.0 ) % m_depth;
	return m_color_map[d];
}

void
Param::info(std::ostream& stat)
{
	stat << RE_START << m_re_start << std::endl;
	stat << RE_END << m_re_end << std::endl;
	stat << IM_START << m_im_start << std::endl;
	stat << IM_END << m_im_end << std::endl;
	stat << DEPTH << m_depth << std::endl;
	stat << FUNCTION << m_function << std::endl;
	stat << WIDTH << m_width << std::endl;
	stat << HEIGHT << m_height << std::endl;
	stat << SAMPLES << m_samples << std::endl;
	stat << COLORING << m_coloring << std::endl;

	if (m_function == 'J') {
		stat << RE_JULIA << m_re_julia << std::endl;
		stat << IM_JULIA << m_im_julia << std::endl;
	}
}

void
Param::save(std::string filename)
{
	std::ofstream stat;

	std::ios_base::iostate exceptionMask = stat.exceptions() | std::ios::failbit | std::ios::badbit;
	stat.exceptions(exceptionMask);

	try {
		stat.open(filename, std::ios::trunc);
		info(stat);
	} catch (std::ios_base::failure& e) {
		g_warning("param: Could not safe %s %d, %s",
			filename.c_str(), errno, strerror(errno));
	}
	stat.close();
}

std::shared_ptr<Param>
Param::open(std::string filename)
{
	auto ret = std::make_shared<Param>(*this); // use colors from actual
	std::ifstream stat;

	std::ios_base::iostate exceptionMask = stat.exceptions() | std::ios::failbit | std::ios::badbit;
	stat.exceptions(exceptionMask);

	try {
		stat.open(filename, std::ios::in);
		bool imend = false;
		while (!stat.eof() &&
			stat.peek() >= 0) {
			std::string line;
			std::getline(stat, line);
			if (line.length() > 6) {
				std::string name;
				std::istringstream is(line);
				is >> name;
				if (line.rfind(RE_START, 0) == 0) {
					is >> ret->m_re_start;
				}
				else if (line.rfind(RE_END, 0) == 0) {
					if (!imend) {
						is >> ret->m_re_end;
						imend = true;
					}
					else { // workaround for bug in first versions
						is >> ret->m_im_end;
					}
				}
				else if (line.rfind(IM_START, 0) == 0) {
					is >> ret->m_im_start;
				}
				else if (line.rfind(IM_END, 0) == 0) {
					is >> ret->m_im_end;
				}
				else if (line.rfind(DEPTH, 0) == 0) {
					is >> ret->m_depth;
				}
				else if (line.rfind(FUNCTION, 0) == 0) {
					is >> ret->m_function;
				}
				else if (line.rfind(WIDTH, 0) == 0) {
					is >> ret->m_width;
				}
				else if (line.rfind(HEIGHT, 0) == 0) {
					is >> ret->m_height;
				}
				else if (line.rfind(SAMPLES, 0) == 0) {
					is >> ret->m_samples;
				}
				else if (line.rfind(COLORING, 0) == 0) {
					//is >> ret->m_coloring;    // yet unused
				}
				else if (line.rfind(RE_JULIA, 0) == 0) {
					is >> ret->m_re_julia;
				}
				else if (line.rfind(IM_JULIA, 0) == 0) {
					is >> ret->m_im_julia;
				}
			}
		}
	} catch (std::ios_base::failure& e) {
		g_warning("param: Could not load %s %d, %s",
			filename.c_str(), errno, strerror(errno));
	}
	stat.close();
	return ret;
}

gint32
Param::get_color_count() const
{
	return m_colors.size();
}

void
Param::set_color(gint32 pos, Gdk::Color color)
{
	m_colors[pos] = color;
}

Gdk::Color
Param::get_color(gint32 pos) const
{
	return m_colors[pos];
}

void
Param::setColoring(enum COLORING coloring)
{
	m_coloring = coloring;
}

enum COLORING
Param::getColoring() const
{
	return m_coloring;
}
