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


#include <iostream>


#include "FractView.h"
#include "Worker.h"

template<class T>
Worker<T>::Worker(FractView* _caller)
: m_caller{_caller}
{
    m_image = std::make_unique<uint32_t[]>(_caller->get_param()->getPixmapWidth());
}

template<class T>
Worker<T>::~Worker()
{
	m_active = false;
}

template<class T> void
Worker<T>::setActive(bool active)
{
	m_active = active;
}

template<class T> std::shared_ptr<Param>
Worker<T>::getParam()
{
    return m_caller->get_param();
}

template<class T> void
Worker<T>::do_work()
{
	auto param = m_caller->get_param();
	T im_step = param->getImStep();
	T re_step = param->getReStep();

	while (m_active) {
		guint row = m_caller->get_row();
		if (row >= param->getPixmapHeight())
			break;

		T im = param->getImStart() + im_step * (T) (row);
		T re = param->getReStart();
		for (guint x = 0; x < param->getPixmapWidth(); ++x) {
			std::complex<T> p(re, im);
			guint n = compute(p);
			//int i = (row * stride + x); // related to image format ARGB32, matches pointer guint32
			// Color mapping test
			//n = (x * row * caller->depth ) / (caller->width * caller->height);
			// This might be the fastest way to color some pixels
			guint32 rgb = param->map_rgb(n, re, im);
			if (m_active)
				m_image[x] = rgb; // no need to convert endianess, as ARGB32 uses always plattform scheme, says the doc
			else
				break;
			re += re_step;
		}
		if (m_active)                                // let raw ptr escape as it just used for this call
			m_caller->notifyRow(row, m_image.get()); // and update display because that is this all about (and for a nice effect do it incrementally)
	}
    m_active = false;
}


template<class T> guint
Worker<T>::compute(std::complex<T> x)
{
	// use complex for parameter passing but internally
	//  use discrete components as this speeds up calculation (2 less multiplications)
	std::complex<T> q;
	prepare(x, q);
	auto param = m_caller->get_param();
	guint depth = param->getDepth();
	guint iter = 0;
	for (; iter < depth; ++iter) {
		std::complex<T> x2(x.real() * x.real(), x.imag() * x.imag());
		if (x2.real() + x2.imag() >= 4.0)
			break;
		T xt = x2.real() - x2.imag();
		x.imag(2.0l * x.real() * x.imag() + q.imag());
		x.real(xt + q.real());
	}
	return iter;
}

template<class T>
JuliaWorker<T>::JuliaWorker(FractView* caller)
: Worker<T>(caller)
{
}

template<class T>
JuliaWorker<T>::~JuliaWorker()
{
}

template<class T> void
JuliaWorker<T>::prepare(std::complex<T> &x, std::complex<T> &q)
{
	auto param = getParam();
	q = std::complex<T>(param->getReJulia(), param->getImJulia());
}

template<class T>
MandelWorker<T>::MandelWorker(FractView* caller)
: Worker<T>(caller)
{
}

template<class T>
MandelWorker<T>::~MandelWorker()
{
}

template<class T> void
MandelWorker<T>::prepare(std::complex<T> &x, std::complex<T> &q)
{
	q = x;
	x = std::complex<T>(0.0, 0.0);
}

template<class T>
NewtonWorker<T>::NewtonWorker(FractView* caller)
: Worker<T>(caller)
{
}

template<class T>
NewtonWorker<T>::~NewtonWorker()
{
}

template<class T> void
NewtonWorker<T>::prepare(std::complex<T> &x, std::complex<T> &q)
{
	q = std::complex<T>(1.0, 0.0);
}

template<class T> std::complex<T>
NewtonWorker<T>::Function(std::complex<T> &z, std::complex<T> &q)
{
	return z * z * z - q;
}

template<class T> std::complex<T>
NewtonWorker<T>::Derivative(std::complex<T> &z)
{
	return std::complex<T>(3.0, 0.0) * z * z;
}

// http://www.wikiwand.com/en/Newton_fractal

template<class T> guint
NewtonWorker<T>::compute(std::complex<T> x)
{
	std::complex<T> q;
	prepare(x, q);
	auto param = getParam();
	guint depth = param->getDepth();
	guint iter = 0;
	std::complex<T> roots[3] = //Roots (solutions) of the polynomial
	{
		std::complex<T>(1.0l, 0),
		std::complex<T>(-.5, std::sqrt(3.0) / 2.0),
		std::complex<T>(-.5, -std::sqrt(3.0) / 2.0)
	};

	T tolerance = 0.000001l;
	for (; iter < depth; ++iter) {
		x -= Function(x, q) / Derivative(x);
		for (guint i = 0; i < sizeof(roots) / sizeof(std::complex<T>); i++) {
			std::complex<T> difference = x - roots[i];
			//If the current iteration is close enough to a root, color the pixel.
			if (abs(difference.real()) < tolerance
				&& abs(difference.imag()) < tolerance) {
				return depth / 3 * (i + 1);
			}
		}
	}
	return 0;
}

// double and long double implementations
template class Worker<double>;
//template class Worker<long double>;

template class JuliaWorker<double>;
//template class JuliaWorker<long double>;

template class MandelWorker<double>;
//template class MandelWorker<long double>;

template class NewtonWorker<double>;
//template class NewtonWorker<long double>;