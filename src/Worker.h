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

#pragma once

#include <memory>
#include <complex>

// a forward declaration is all we need here
class FractView;
class Param;

template<class T>
class Worker {
public:
    Worker(FractView* caller);
    virtual ~Worker();

    void do_work();

    virtual guint compute(std::complex<T> p);
    virtual void prepare(std::complex<T> &x, std::complex<T> &q) = 0;
    std::shared_ptr<Param> getParam();
    unsigned int* get_image();
    void setActive(bool active);
protected:
    FractView* m_caller;
    bool m_active{true};
    unsigned int* m_image;
private:
};

template<class T>
class JuliaWorker : public Worker<T> {
public:
    JuliaWorker(FractView* caller);
    virtual ~JuliaWorker();

    void prepare(std::complex<T> &x, std::complex<T> &q) override;
    using Worker<T>::getParam;
private:

};

template<class T>
class MandelWorker : public Worker<T> {
public:
    MandelWorker(FractView* caller);
    virtual ~MandelWorker();

    void prepare(std::complex<T> &x, std::complex<T> &q) override;
    using Worker<T>::getParam;
private:

};

template<class T>
class NewtonWorker : public Worker<T> {
public:
    NewtonWorker(FractView* caller);
    virtual ~NewtonWorker();

    void prepare(std::complex<T> &x, std::complex<T> &q) override;
    guint compute(std::complex<T> x) override;
    std::complex<T> Function(std::complex<T> &z, std::complex<T> &q);
    std::complex<T> Derivative(std::complex<T> &z);
    using Worker<T>::getParam;
private:

};
