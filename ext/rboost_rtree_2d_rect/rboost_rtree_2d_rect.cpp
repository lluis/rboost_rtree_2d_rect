// file: rboost_rtree_2d_rect.cpp
// note: source code is indented with tabs, tab-width=2

// Ruby bindings for boost C++ library, support for
// geometry/spatial_indexes/rtree
// http://en.wikipedia.org/wiki/R-tree
// http://www.boost.org/
// http://www.boost.org/doc/libs/1_54_0/libs/geometry/doc/html/geometry/spatial_indexes/introduction.html
// http://media.pragprog.com/titles/ruby3/ext_ruby.pdf
// /usr/share/doc/ruby-2.0.0_p247-r1/README.EXT.bz2
// http://www.angelfire.com/electronic2/issac/rb_cpp_ext_tut.txt
//
// c Stefan Salewsk, mail@ssalewski.de
// License GPL
// Version 0.1 15-SEP-2013
// tested for Ruby 1.9.3 and Ruby 2.0 -- not much yet

// This version stores rectangles in 2d -- I intent to make a modified version which stores plain points.
// Modifications for 3D should be easy.
// Currently I have no idea how we can make a fully generic version for arbitrary dimensions.

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>

// to store queries results
#include <vector>

// just for output
//#include <iostream>
#include <boost/foreach.hpp>

// order of includes does matter -- with ruby.h at the top we get strange errors
#include "ruby.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 2, bg::cs::cartesian> point;
typedef bg::model::box<point> box;
typedef std::pair<box, VALUE> RTE; // R-tree Entry

VALUE cRT2dR;
VALUE cRD2dR;

// R-tree Class
class RTC : public bgi::rtree< RTE, bgi::quadratic<16> >
{
public:
	VALUE hash; // supported by Ruby hash
};

static void rtree_del(void* t)
{
	delete (RTC*) t;
}

static void rtree_mark(void *p)
{
	RTC *rt = (RTC*) p;
	rb_gc_mark(rt->hash);
}

static VALUE rtree_alloc(VALUE klass)
{
	RTC *rt = new RTC;
	return Data_Wrap_Struct(klass, rtree_mark, rtree_del, rt);
}

static VALUE rtree_init(int argc, VALUE* argv, VALUE self)
{
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	rt->hash = rb_hash_new();
	return self;
}

static void rtree_value_free(void* v)
{
	delete (RTE*) v;
}

static VALUE rtree_insert(VALUE self, VALUE obj, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	if (NIL_P(rb_hash_aref(rt->hash, obj)))
	{
		box b(point(NUM2DBL(x1), NUM2DBL(y1)), point(NUM2DBL(x2), NUM2DBL(y2)));
		RTE* v = new RTE;
		*v = std::make_pair(b, obj);
		VALUE h = Data_Wrap_Struct(cRD2dR, 0, rtree_value_free, v);
		rb_hash_aset(rt->hash, obj, h);
		rt->insert(*v);
	}
	else
		rb_raise(rb_eRuntimeError, "object is already inserted!");
	return Qnil;
}

static VALUE rtree_update(VALUE self, VALUE obj, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	RTC *rt;
	RTE* v;
	VALUE o;
	Data_Get_Struct(self, RTC, rt);
	if (NIL_P(o = rb_hash_aref(rt->hash, obj)))
		rb_raise(rb_eRuntimeError, "object does not exist in R-tree!");
	Data_Get_Struct(o, RTE, v);
	rt->remove(*v);
	box b(point(NUM2DBL(x1), NUM2DBL(y1)), point(NUM2DBL(x2), NUM2DBL(y2)));
	v = new RTE;
	*v = std::make_pair(b, obj);
	VALUE h = Data_Wrap_Struct(cRD2dR, 0, rtree_value_free, v);
	rb_hash_aset(rt->hash, obj, h);
	rt->insert(*v);
	return Qnil;
}

static VALUE rtree_update_or_insert(VALUE self, VALUE obj, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	RTC *rt;
	RTE* v;
	VALUE o;
	Data_Get_Struct(self, RTC, rt);
	if (!NIL_P(o = rb_hash_aref(rt->hash, obj)))
	{
		Data_Get_Struct(o, RTE, v);
		rt->remove(*v);
	}
	box b(point(NUM2DBL(x1), NUM2DBL(y1)), point(NUM2DBL(x2), NUM2DBL(y2)));
	v = new RTE;
	*v = std::make_pair(b, obj);
	VALUE h = Data_Wrap_Struct(cRD2dR, 0, rtree_value_free, v);
	rb_hash_aset(rt->hash, obj, h);
	rt->insert(*v);
	return ((NIL_P(o)) ? Qtrue : Qfalse);
}

static VALUE rtree_update_insert(VALUE self, VALUE obj, VALUE a)
{
	if (TYPE(a) != T_ARRAY) //|| (rb_ary_len(a) != 4))
		rb_raise(rb_eArgError, "Array with 4 numeric values expected!");
	VALUE y2 = rb_ary_pop(a);
	VALUE x2 = rb_ary_pop(a);
	VALUE y1 = rb_ary_pop(a);
	VALUE x1 = rb_ary_pop(a);
	return 	rtree_update_or_insert(self, obj, x1, y1, x2, y2);
}

static VALUE rtree_rect(VALUE self, VALUE obj)
{
	RTC *rt;
	RTE* v;
	Data_Get_Struct(self, RTC, rt);
	VALUE h = rb_hash_aref(rt->hash, obj);
	if (NIL_P(h)) return Qnil;
	Data_Get_Struct(h, RTE, v);
	return rb_ary_new3(4,
		rb_float_new(v->first.min_corner().get<0>()),
		rb_float_new(v->first.min_corner().get<1>()),
		rb_float_new(v->first.max_corner().get<0>()),
		rb_float_new(v->first.max_corner().get<1>()));
}

static VALUE rtree_remove(VALUE self, VALUE obj)
{
	RTC *rt;
	RTE* v;
	VALUE o;
	Data_Get_Struct(self, RTC, rt);
	if (NIL_P(o = rb_hash_aref(rt->hash, obj)))
		return Qfalse;
	else
	{
		Data_Get_Struct(o, RTE, v);
		rt->remove(*v);
 		rb_hash_delete(rt->hash, obj);
		return Qtrue;
	}
}

static VALUE rtree_include(VALUE self, VALUE obj)
{
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	return (NIL_P(rb_hash_aref(rt->hash, obj)) ? Qfalse : Qtrue);
}

//static VALUE rtree_length(VALUE self)
//{
//	RTC *rt;
//	Data_Get_Struct(self, RTC, rt);
//	return rb_hash_size(rt->hash); // not available
//}

static VALUE rtree_intersects(VALUE self, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	box query_box(point(NUM2DBL(x1), NUM2DBL(y1)), point(NUM2DBL(x2), NUM2DBL(y2)));
	std::vector<RTE> result;
	rt->query(bgi::intersects(query_box), std::back_inserter(result));
	VALUE a = rb_ary_new();
	BOOST_FOREACH(RTE const& v, result)
		rb_ary_push(a, v.second);
	return a;
}

static VALUE rtree_intersects_each(VALUE self, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	if (!rb_block_given_p())
		rb_raise(rb_eArgError, "block expected!");
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	box query_box(point(NUM2DBL(x1), NUM2DBL(y1)), point(NUM2DBL(x2), NUM2DBL(y2)));
	std::vector<RTE> result;
	rt->query(bgi::intersects(query_box), std::back_inserter(result));
	BOOST_FOREACH(RTE const& v, result)
		rb_yield(v.second);
	return Qnil;
}

static VALUE rtree_intersects_rect(VALUE self, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	box query_box(point(NUM2DBL(x1), NUM2DBL(y1)), point(NUM2DBL(x2), NUM2DBL(y2)));
	std::vector<RTE> result;
	rt->query(bgi::intersects(query_box), std::back_inserter(result));
	VALUE a = rb_ary_new();
	BOOST_FOREACH(RTE const& v, result)
	{
		VALUE arr = rb_ary_new3(5, v.second, 
		rb_float_new(v.first.min_corner().get<0>()),
		rb_float_new(v.first.min_corner().get<1>()),
		rb_float_new(v.first.max_corner().get<0>()),
		rb_float_new(v.first.max_corner().get<1>()));
		rb_ary_push(a, arr);
	}
	return a;
}

static VALUE rtree_intersects_rect_each(VALUE self, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	if (!rb_block_given_p())
		rb_raise(rb_eArgError, "block expected!");
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	box query_box(point(NUM2DBL(x1), NUM2DBL(y1)), point(NUM2DBL(x2), NUM2DBL(y2)));
	std::vector<RTE> result;
	rt->query(bgi::intersects(query_box), std::back_inserter(result));
	BOOST_FOREACH(RTE const& v, result)
	{
		VALUE arr = rb_ary_new3(5, v.second, 
		rb_float_new(v.first.min_corner().get<0>()),
		rb_float_new(v.first.min_corner().get<1>()),
		rb_float_new(v.first.max_corner().get<0>()),
		rb_float_new(v.first.max_corner().get<1>()));
		rb_yield(arr);
	}
	return Qnil;
}

static VALUE rtree_nearest(VALUE self, VALUE x, VALUE y, VALUE k)
{
	long kk;
	if ((kk = NUM2LONG(k)) <= 0)
		rb_raise(rb_eArgError, "FIXNUM > 0 expected!");
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	std::vector<RTE> result;
	rt->query(bgi::nearest(point(NUM2DBL(x), NUM2DBL(y)), kk), std::back_inserter(result));
	VALUE a = rb_ary_new();
	BOOST_FOREACH(RTE const& v, result)
		rb_ary_push(a, v.second);
	return a;
}

static VALUE rtree_nearest_rect(VALUE self, VALUE x, VALUE y, VALUE k)
{
	long kk;
	if ((kk = NUM2LONG(k)) <= 0)
		rb_raise(rb_eArgError, "FIXNUM > 0 expected!");
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	std::vector<RTE> result;
	rt->query(bgi::nearest(point(NUM2DBL(x), NUM2DBL(y)), kk), std::back_inserter(result));
	VALUE a = rb_ary_new();
	BOOST_FOREACH(RTE const& v, result)
	{
		VALUE arr = rb_ary_new3(5, v.second, 
		rb_float_new(v.first.min_corner().get<0>()),
		rb_float_new(v.first.min_corner().get<1>()),
		rb_float_new(v.first.max_corner().get<0>()),
		rb_float_new(v.first.max_corner().get<1>()));
		rb_ary_push(a, arr);
	}
	return a;
}

static VALUE rtree_nearest_each(VALUE self, VALUE x, VALUE y, VALUE k)
{
	long kk;
	if ((kk = NUM2LONG(k)) <= 0)
		rb_raise(rb_eArgError, "FIXNUM > 0 expected!");
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	std::vector<RTE> result;
	rt->query(bgi::nearest(point(NUM2DBL(x), NUM2DBL(y)), kk), std::back_inserter(result));
	BOOST_FOREACH(RTE const& v, result)
		rb_yield(v.second);
	return Qnil;
}

static VALUE rtree_nearest_rect_each(VALUE self, VALUE x, VALUE y, VALUE k)
{
	long kk;
	if ((kk = NUM2LONG(k)) <= 0)
		rb_raise(rb_eArgError, "FIXNUM > 0 expected!");
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	std::vector<RTE> result;
	rt->query(bgi::nearest(point(NUM2DBL(x), NUM2DBL(y)), kk), std::back_inserter(result));
	BOOST_FOREACH(RTE const& v, result)
	{
		VALUE arr = rb_ary_new3(5, v.second, 
		rb_float_new(v.first.min_corner().get<0>()),
		rb_float_new(v.first.min_corner().get<1>()),
		rb_float_new(v.first.max_corner().get<0>()),
		rb_float_new(v.first.max_corner().get<1>()));
		rb_yield(arr);

	}
	return Qnil;
}

typedef int (ruby_foreach_method)(...);

static int each_obj_i(VALUE key, VALUE val)
{
	rb_yield(key);
	return ST_CONTINUE;
}

static VALUE rtree_each_object(VALUE self)
{
	if (!rb_block_given_p())
		rb_raise(rb_eArgError, "block expected!");
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	rb_hash_foreach(rt->hash, (ruby_foreach_method*) &each_obj_i, 0);
	return self;
}

static int each_pair_i(VALUE key, VALUE val)
{
	RTE* v;
	Data_Get_Struct(val, RTE, v);
	VALUE a = rb_ary_new3(5, key,
		rb_float_new(v->first.min_corner().get<0>()),
		rb_float_new(v->first.min_corner().get<1>()),
		rb_float_new(v->first.max_corner().get<0>()),
		rb_float_new(v->first.max_corner().get<1>()));
	rb_yield(a);
	return ST_CONTINUE;
}

static VALUE rtree_each_pair(VALUE self)
{
	if (!rb_block_given_p())
		rb_raise(rb_eArgError, "block expected!");
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	rb_hash_foreach(rt->hash, (ruby_foreach_method*) &each_pair_i, 0);
	return self;
}

static int to_a_i(VALUE key, VALUE val, VALUE ary)
{
	RTE* v;
	Data_Get_Struct(val, RTE, v);
	VALUE a = rb_ary_new3(5, key,
		rb_float_new(v->first.min_corner().get<0>()),
		rb_float_new(v->first.min_corner().get<1>()),
		rb_float_new(v->first.max_corner().get<0>()),
		rb_float_new(v->first.max_corner().get<1>()));
		rb_ary_push(ary, a);
		return ST_CONTINUE;
}

static VALUE rtree_to_a(VALUE self)
{
	RTC *rt;
	Data_Get_Struct(self, RTC, rt);
	VALUE ary = rb_ary_new();
	rb_hash_foreach(rt->hash, (ruby_foreach_method*) &to_a_i, ary);
	//OBJ_INFECT(ary, self);
	return ary;
}

typedef VALUE (ruby_method)(...);

extern "C" void Init_rboost_rtree_2d_rect() {
	VALUE mBOOST = rb_define_module("BOOST");
	cRT2dR = rb_define_class_under(mBOOST, "R_tree_2d_rect", rb_cObject);
	rb_define_alloc_func(cRT2dR, rtree_alloc);
	rb_define_method(cRT2dR, "initialize", (ruby_method*) &rtree_init, -1);
	rb_define_method(cRT2dR, "insert", (ruby_method*) &rtree_insert, 5);
	rb_define_method(cRT2dR, "update", (ruby_method*) &rtree_update, 5);
	rb_define_method(cRT2dR, "remove", (ruby_method*) &rtree_remove, 1);
	rb_define_method(cRT2dR, "each_pair", (ruby_method*) &rtree_each_pair, 0);
	rb_define_method(cRT2dR, "each_object", (ruby_method*) &rtree_each_object, 0);
	rb_define_method(cRT2dR, "to_a", (ruby_method*) &rtree_to_a, 0);
	rb_define_alias(cRT2dR, "delete", "remove");
	rb_define_method(cRT2dR, "update_or_insert", (ruby_method*) &rtree_update_or_insert, 5);
	rb_define_method(cRT2dR, "[]=", (ruby_method*) &rtree_update_insert, 2);
	rb_define_method(cRT2dR, "intersects?", (ruby_method*) &rtree_intersects, 4);
	rb_define_method(cRT2dR, "intersects_each?", (ruby_method*) &rtree_intersects_each, 4);
	rb_define_method(cRT2dR, "intersects_rect?", (ruby_method*) &rtree_intersects_rect, 4);
	rb_define_method(cRT2dR, "intersects_rect_each?", (ruby_method*) &rtree_intersects_rect_each, 4);
	rb_define_method(cRT2dR, "include?", (ruby_method*) &rtree_include, 1);
	rb_define_method(cRT2dR, "rect", (ruby_method*) &rtree_rect, 1);
	rb_define_alias(cRT2dR, "[]", "rect");
	rb_define_method(cRT2dR, "nearest", (ruby_method*) &rtree_nearest, 3);
	rb_define_method(cRT2dR, "nearest_each", (ruby_method*) &rtree_nearest_each, 3);
	rb_define_method(cRT2dR, "nearest_rect", (ruby_method*) &rtree_nearest_rect, 3);
	rb_define_method(cRT2dR, "nearest_rect_each", (ruby_method*) &rtree_nearest_rect_each, 3);
}

