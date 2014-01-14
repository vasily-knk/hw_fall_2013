#include "stdafx.h"
#include "primitives.h"
#include "segment_windowing.h"
#include "visualization/viewer_adapter.h"
#include "visualization/draw_util.h"

void range_test()
{
    vector<point_t> points;
    vector<range_t> ranges;


    for (size_t i = 0; i < 1000; ++i)
    {
        const coord_t x = rand();
        const coord_t y = rand();
        points.push_back(point_t(x, y));
        points.push_back(point_t(x, rand()));
    }


    for (size_t i = 0; i < 1000; ++i)
    {
        coord_t inf = rand();
        coord_t sup = rand();
        if (sup < inf)
            std::swap(inf, sup);

        ranges.push_back(range_t(inf, sup));
    }



/*
    for (size_t i = 0; i < 10; ++i)
        points.push_back(point_t(i,  5));
*/


    const range_tree_t range_tree(points);

//    auto ind = range_tree.query(range_t(2, 8), range_t(4, 9));

    vector<bool> returned;
    for (size_t i = 0; i < ranges.size() / 2; ++i)
    {
        returned.resize(range_tree.points().size(), false);
        
        range_t x_range = ranges.at(i * 2 + 0);
        range_t y_range = ranges.at(i * 2 + 1);

        const auto indices = range_tree.query(x_range, y_range);
        BOOST_FOREACH(const auto index, indices)
        {
            returned.at(index) = true;
            const point_t point = range_tree.points().at(index);
            MY_ASSERT(point.x >= x_range.inf && point.x < x_range.sup && point.y >= y_range.inf && point.y < y_range.sup);
        }

        for (size_t index = 0; index < range_tree.points().size(); ++index)
        {
            const point_t point = range_tree.points().at(index);
            if (!returned.at(index))
                MY_ASSERT(point.x < x_range.inf || point.x >= x_range.sup || point.y < y_range.inf || point.y >= y_range.sup);
        }
    }
}

void segment_test()
{
    vector<segment_t> segments;
    segments.push_back(segment_t(point_t(0, 0), point_t(100, 20)));
    segments.push_back(segment_t(point_t(0, 50), point_t(50, 20)));
    segments.push_back(segment_t(point_t(40, 40), point_t(100, 40)));
    windowing_t t(segments);

    auto result = t.query(range_t(35, 105), range_t(35, 45));
    BOOST_FOREACH(auto index, result)
        cout << index << endl;

};


namespace visualization
{

    struct segment_tree_viewer
        : viewer_adapter    
    {
        void draw(drawer_type & drawer) const override
        {
            for (size_t i = 0; i < segments_.size(); ++i)
            {
                drawer.set_color(indices_.count(i) ? Qt::red : Qt::blue);
                draw_segment(drawer, segments_.at(i));
            }

            if (new_segment_)
            {
                drawer.set_color(Qt::cyan);
                draw_segment(drawer, *new_segment_);
            }

            if (window_)
            {
                drawer.set_color(Qt::green);
                draw_rect(drawer, *window_);
            }
            
            if (old_window_)
            {
                drawer.set_color(Qt::darkGreen);
                draw_rect(drawer, *old_window_);
            }
        }

        bool on_press(point_type const & pt) override
        {
            if (window_)
            {
                window_click(pt);
            }
            else if (!new_segment_)
            {
                new_segment_ = segment_t(pt, pt);
            }
            else
            {
                segments_.push_back(*new_segment_);
                new_segment_.reset();
                windowing_.reset();
            }

            return true;
        }

        bool on_key(int key)
        {
            switch(key)
            {
            case Qt::Key_Escape:
                window_.reset();
                new_segment_.reset();
                break;
            case Qt::Key_Space:
                window_click(last_mouse_);
                break;

            default:
                return false;
            }
            return true;
        }

        bool on_move(point_type const &pt) override
        {
            last_mouse_ = pt;
            if (window_)
            {
                (*window_)[1] = pt;
                update_indices(*window_);
                return true;
            }
            else if (new_segment_)
            {
                (*new_segment_)[1] = pt;
                return true;
            }
            return false;
        }
    private:
        void draw_segment(drawer_type & drawer, const segment_t &s) const
        {
            drawer.draw_point(s[0], 3);
            drawer.draw_point(s[1], 3);
            drawer.draw_line(s, 1);
        }

        void draw_rect(drawer_type & drawer, const segment_t &s) const 
        {
            const point_t p1 = s[0];
            const point_t p2 = point_t(s[0].x, s[1].y);
            const point_t p3 = s[1];
            const point_t p4 = point_t(s[1].x, s[0].y);

            drawer.draw_line(p1, p2);
            drawer.draw_line(p2, p3);
            drawer.draw_line(p3, p4);
            drawer.draw_line(p4, p1);
        }

        void window_click(point_type const &pt)
        {
            new_segment_.reset();
            if (window_)
            {
                old_window_ = window_;
                window_.reset();
            }
            else
            {
                window_ = segment_t(pt, pt);
                build_struct();
                old_window_.reset();
            }
        }

        void build_struct()
        {
            if (windowing_)
                return;

            windowing_ = boost::make_shared<windowing_t>(segments_);
        }

        void update_indices(const segment_t &s)
        {
            indices_ = windowing_->query(x_range(s), y_range(s));
        }

    private:
        vector<segment_t> segments_; 
        optional<segment_t> new_segment_;
        optional<segment_t> window_, old_window_;
        point_t last_mouse_;

        shared_ptr<windowing_t> windowing_;
        windowing_t::indices_t indices_;
    };
}



void main(int argc, char** argv)
{
    QApplication app(argc, argv);
    visualization::segment_tree_viewer viewer;
    visualization::run_viewer(&viewer, "Segment tree");
    //range_test();
}
