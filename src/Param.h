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
#include <vector>

enum class Coloring {
    Strech,
    Repeat
};

enum class Function : gchar {
      Mandelbrot = 'M'
    , Juliaset = 'J'
    , Newton = 'N'
};

class Param {
public:
    Param();
    Param(const Param& orig);
    virtual ~Param() = default;

    void setDepth(guint depth);
    guint getDepth();
    void setFunction(enum Function function);
    enum Function getFunction();
    void setWidth(guint width);
    guint getWidth();
    void setHeight(guint height);
    guint getHeight();
    guint getPixmapWidth();
    guint getPixmapHeight();
    double getImStart();
    double getReStart();
    double getReJulia();
    double getImJulia();
    void setReJulia(double reJulia);
    void setImJulia(double imJulia);
    void setReStart(double reStart);
    void setReEnd(double reEnd);
    void setImStart(double imStart);
    void setImEnd(double imEnd);

    double getImStep();
    double getReStep();

    std::shared_ptr<Param> update(Cairo::RectangleInt saveRect);
    void build_color_map();
    std::shared_ptr<Param> open(std::string filename);

    guint32 map_rgb(guint32 d, long double re, long double im);
    bool isPrimaryWindow();
    void setPrimaryWindow(bool primary);
    void setSamples(guint32 samp);
    guint32 getSamples() const;
    void info(std::ostream& stat);
    void save(std::string filename);
    void set_color(gint32 pos, Gdk::Color color);
    Gdk::Color get_color(gint32 pos) const;
    gint32 get_color_count() const;
    void setColoring(enum Coloring coloring);
    enum Coloring getColoring() const;
    
    static constexpr auto RE_START{"reStart: "};
    static constexpr auto RE_END{"reEnd: "};
    static constexpr auto IM_START{"imStart: "};
    static constexpr auto IM_END{"imEnd: "};
    static constexpr auto DEPTH{"depth: "};
    static constexpr auto FUNCTION{"function: "};
    static constexpr auto WIDTH{"width: "};
    static constexpr auto HEIGHT{"height: "};
    static constexpr auto SAMPLES{"samples: "};
    static constexpr auto COLORING{"coloring: "};
    static constexpr auto RE_JULIA{"reJulia: "};
    static constexpr auto IM_JULIA{"imJulia: "};    
    
    static enum Function char2Function(guint32 function);
private:
    enum Function m_function;
    guint32 m_samples; // Multisampling used on x&y so 4 will result in 16 samples per display pixel
    guint m_width;
    guint m_height;
    guint m_depth;
    double m_re_start;
    double m_re_end;
    double m_im_start;
    double m_im_end;
    double m_re_julia;
    double m_im_julia;
    std::vector<guint32> m_color_map;
    bool m_primaryWindow;
    std::vector<Gdk::Color> m_colors;
    enum Coloring m_coloring;

    guint32 map(gint32 d, double depth_step);
};
