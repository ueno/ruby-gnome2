/* -*- c-file-style: "ruby"; indent-tabs-mode: nil -*- */
/*
 *  Copyright (C) 2012-2015  Ruby-GNOME2 Project Team
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#include "rb-gi-private.h"

#define RG_TARGET_NAMESPACE rb_cGILoader

static const gchar *boxed_class_converters_name = "@@boxed_class_converters";

static VALUE
rg_s_define_class(int argc, VALUE *argv, G_GNUC_UNUSED VALUE klass)
{
    VALUE rb_class;
    VALUE rb_gtype, rb_name, rb_module;
    VALUE rb_options, rb_parent, rb_size;
    GType gtype;

    rb_scan_args(argc, argv, "31", &rb_gtype, &rb_name, &rb_module, &rb_options);
    rbg_scan_options(rb_options,
                     "parent", &rb_parent,
                     "size", &rb_size,
                     NULL);

    gtype = NUM2ULONG(rb_to_int(rb_gtype));
    rb_class = G_DEF_CLASS_WITH_PARENT(gtype, RVAL2CSTR(rb_name),
                                       rb_module, rb_parent);
    if (!NIL_P(rb_size)) {
        rb_iv_set(rb_class, "@size", rb_size);
    }
    return rb_class;
}

static VALUE
rg_s_define_interface(G_GNUC_UNUSED VALUE klass,
                      VALUE rb_gtype, VALUE rb_name, VALUE rb_module)
{
    GType gtype;

    gtype = NUM2ULONG(rb_to_int(rb_gtype));
    return G_DEF_INTERFACE(gtype, RVAL2CSTR(rb_name), rb_module);
}

static VALUE
struct_alloc(VALUE klass)
{
    VALUE rb_size;
    size_t size;
    gpointer instance;

    rb_size = rb_iv_get(klass, "@size");
    size = NUM2ULONG(rb_size);
    instance = xmalloc(size);
    memset(instance, 0, size);
    return Data_Wrap_Struct(klass, NULL, xfree, instance);
}

static VALUE
rg_s_define_struct(int argc, VALUE *argv, G_GNUC_UNUSED VALUE klass)
{
    VALUE rb_size, rb_name, rb_module;
    VALUE rb_options, rb_parent;
    VALUE rb_class;

    rb_scan_args(argc, argv, "31", &rb_size, &rb_name, &rb_module, &rb_options);
    rbg_scan_options(rb_options,
                     "parent", &rb_parent,
                     NULL);

    rb_size = rb_to_int(rb_size);
    if (NIL_P(rb_parent)) {
        rb_parent = rb_cObject;
    }
    rb_class = rb_define_class_under(rb_module, RVAL2CSTR(rb_name), rb_parent);
    rb_iv_set(rb_class, "@size", rb_size);
    rb_define_alloc_func(rb_class, struct_alloc);
    return rb_class;
}

static VALUE
rg_s_define_error(int argc, VALUE *argv, G_GNUC_UNUSED VALUE klass)
{
    VALUE rb_domain, rb_name, rb_module;
    VALUE rb_options, rb_parent, rb_gtype;
    GQuark domain;
    const gchar *name;
    GType gtype = G_TYPE_INVALID;

    rb_scan_args(argc, argv, "31",
                 &rb_domain, &rb_name, &rb_module, &rb_options);
    rbg_scan_options(rb_options,
                     "parent", &rb_parent,
                     "gtype", &rb_gtype,
                     NULL);

    if (RB_TYPE_P(rb_domain, RUBY_T_STRING)) {
        domain = g_quark_from_string(RVAL2CSTR(rb_domain));
        if (domain == 0) {
            rb_raise(rb_eArgError,
                     "invalid domain name: <%s>",
                     rbg_inspect(rb_domain));
        }
    } else {
        domain = NUM2UINT(rb_domain);
    }

    name = RVAL2CSTR(rb_name);

    if (NIL_P(rb_parent)) {
        rb_parent = rb_eStandardError;
    }

    if (!NIL_P(rb_gtype)) {
        gtype = NUM2ULONG(rb_funcall(rb_gtype, rb_intern("to_i"), 0));
    }

    return G_DEF_ERROR(domain, name, rb_module, rb_parent, gtype);
}

typedef struct {
    GType type;
    VALUE rb_converters;
    VALUE rb_converter;
} BoxedInstance2RObjData;

static void
boxed_class_converter_free(gpointer user_data)
{
    BoxedInstance2RObjData *data = user_data;
    rb_ary_delete(data->rb_converters, data->rb_converter);
    g_free(data);
}

static VALUE
boxed_instance2robj(gpointer instance, gpointer user_data)
{
    BoxedInstance2RObjData *data = user_data;
    VALUE default_rb_instance;
    VALUE klass;
    gint flags = 0;
    ID id_call;

    default_rb_instance = rbgobj_make_boxed_default(instance, data->type);
    CONST_ID(id_call, "call");
    klass = rb_funcall(data->rb_converter, id_call, 1, default_rb_instance);
    return rbgobj_make_boxed_raw(instance, data->type, klass, flags);
}

static VALUE
rg_s_register_boxed_class_converter(VALUE klass, VALUE rb_gtype)
{
    RGConvertTable table;
    BoxedInstance2RObjData *data;
    ID id_to_i;
    VALUE boxed_class_converters;

    memset(&table, 0, sizeof(RGConvertTable));
    CONST_ID(id_to_i, "to_i");
    table.type = NUM2ULONG(rb_funcall(rb_gtype, id_to_i, 0));
    table.klass = Qnil;
    table.instance2robj = boxed_instance2robj;

    data = g_new(BoxedInstance2RObjData, 1);
    data->type = table.type;
    data->rb_converter = rb_block_proc();
    boxed_class_converters = rb_cv_get(klass, boxed_class_converters_name);
    rb_ary_push(boxed_class_converters, data->rb_converter);
    table.user_data = data;
    table.notify = boxed_class_converter_free;

    rbgobj_convert_define(&table);

    return Qnil;
}

static VALUE
rg_s_register_constant_rename_map(G_GNUC_UNUSED VALUE klass,
                                  VALUE rb_original,
                                  VALUE rb_renamed)
{
    G_RENAME_CONSTANT(RVAL2CSTR(rb_original), RVAL2CSTR(rb_renamed));
    return Qnil;
}

static VALUE
rg_s_start_callback_dispatch_thread(G_GNUC_UNUSED VALUE klass)
{
    rbgutil_start_callback_dispatch_thread();
    return Qnil;
}

static VALUE
rg_s_reference_gobject(int argc, VALUE *argv, G_GNUC_UNUSED VALUE klass)
{
    VALUE rb_gobject;
    VALUE rb_options, rb_sink;
    gpointer gobject;
    gboolean sink;

    rb_scan_args(argc, argv, "11",
                 &rb_gobject, &rb_options);
    rbg_scan_options(rb_options,
                     "sink", &rb_sink,
                     NULL);

    gobject = RVAL2GOBJ(rb_gobject);

    if (NIL_P(rb_sink)) {
        sink = FALSE;
    } else {
        sink = RVAL2CBOOL(rb_sink);
    }

    if (sink) {
        g_object_ref_sink(gobject);
    } else {
        g_object_ref(gobject);
    }

    return Qnil;
}

void
rb_gi_loader_init(VALUE rb_mGI)
{
    VALUE RG_TARGET_NAMESPACE;

    RG_TARGET_NAMESPACE = rb_define_class_under(rb_mGI, "Loader", rb_cObject);

    rb_cv_set(RG_TARGET_NAMESPACE, boxed_class_converters_name, rb_ary_new());

    RG_DEF_SMETHOD(define_class, -1);
    RG_DEF_SMETHOD(define_interface, 3);
    RG_DEF_SMETHOD(define_struct, -1);
    RG_DEF_SMETHOD(define_error, -1);
    RG_DEF_SMETHOD(register_boxed_class_converter, 1);
    RG_DEF_SMETHOD(register_constant_rename_map, 2);
    RG_DEF_SMETHOD(start_callback_dispatch_thread, 0);
    RG_DEF_SMETHOD(reference_gobject, -1);
}
