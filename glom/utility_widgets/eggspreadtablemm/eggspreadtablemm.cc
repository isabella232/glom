/*
 *
 * Copyright 2010 The gtkmm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <glom/utility_widgets/eggspreadtablemm/eggspreadtablemm.h>
#include <glom/utility_widgets/eggspreadtablemm/private/eggspreadtablemm_p.h>
#include <glibmm/exceptionhandler.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <glom/utility_widgets/eggspreadtable/eggspreadtable.h>

namespace Egg
{

void SpreadTable::append_child(Widget& widget)
{
  insert_child(widget, -1 /* see the C documentation */);
}

} // namespace Egg

namespace Glib
{

Egg::SpreadTable* wrap(EggSpreadTable* object, bool take_copy)
{
  return dynamic_cast<Egg::SpreadTable *> (Glib::wrap_auto ((GObject*)(object), take_copy));
}

} /* namespace Glib */

namespace Egg
{


/* The *_Class implementation: */

const Glib::Class& SpreadTable_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Class has to know the class init function to clone custom types.
    class_init_func_ = &SpreadTable_Class::class_init_function;

    // This is actually just optimized away, apparently with no harm.
    // Make sure that the parent type has been created.
    //CppClassParent::CppObjectType::get_type();

    // Create the wrapper type, with the same class/instance size as the base type.
    register_derived_type(egg_spread_table_get_type());

    // Add derived versions of interfaces, if the C type implements any interfaces:
    Gtk::Orientable::add_interface(get_type());
  }

  return *this;
}


void SpreadTable_Class::class_init_function(void* g_class, void* class_data)
{
  BaseClassType *const klass = static_cast<BaseClassType*>(g_class);
  CppClassParent::class_init_function(klass, class_data);


}


Glib::ObjectBase* SpreadTable_Class::wrap_new(GObject* o)
{
  return manage(new SpreadTable((EggSpreadTable*)(o)));

}


/* The implementation: */

SpreadTable::SpreadTable(const Glib::ConstructParams& construct_params)
:
  Gtk::Container(construct_params)
{
  }

SpreadTable::SpreadTable(EggSpreadTable* castitem)
:
  Gtk::Container((GtkContainer*)(castitem))
{
  }

SpreadTable::~SpreadTable()
{
  destroy_();
}

SpreadTable::CppClassType SpreadTable::spreadtable_class_; // initialize static member

GType SpreadTable::get_type()
{
  return spreadtable_class_.init().get_type();
}


GType SpreadTable::get_base_type()
{
  return egg_spread_table_get_type();
}


SpreadTable::SpreadTable()
:
  // Mark this class as non-derived to allow C++ vfuncs to be skipped.
  Glib::ObjectBase(nullptr),
  Gtk::Container(Glib::ConstructParams(spreadtable_class_.init()))
{


}

SpreadTable::SpreadTable(Gtk::Orientation orientation, guint lines)
:
  // Mark this class as non-derived to allow C++ vfuncs to be skipped.
  Glib::ObjectBase(nullptr),
  Gtk::Container(Glib::ConstructParams(spreadtable_class_.init(), "orientation", ((GtkOrientation)(orientation)), "lines", lines, static_cast<char*>(nullptr)))
{


}

void SpreadTable::insert_child(Gtk::Widget& widget, int index)
{
egg_spread_table_insert_child(gobj(), (widget).gobj(), index);
}

guint SpreadTable::get_child_line(const Gtk::Widget& child, int size) const
{
  return egg_spread_table_get_child_line(const_cast<EggSpreadTable*>(gobj()), const_cast<GtkWidget*>(child.gobj()), size);
}

void SpreadTable::set_lines(guint lines)
{
egg_spread_table_set_lines(gobj(), lines);
}

guint SpreadTable::get_lines() const
{
  return egg_spread_table_get_lines(const_cast<EggSpreadTable*>(gobj()));
}

void SpreadTable::set_vertical_spacing(guint spacing)
{
egg_spread_table_set_vertical_spacing(gobj(), spacing);
}

guint SpreadTable::get_vertical_spacing() const
{
  return egg_spread_table_get_vertical_spacing(const_cast<EggSpreadTable*>(gobj()));
}

void SpreadTable::set_horizontal_spacing(guint spacing)
{
egg_spread_table_set_horizontal_spacing(gobj(), spacing);
}

guint SpreadTable::get_horizontal_spacing() const
{
  return egg_spread_table_get_horizontal_spacing(const_cast<EggSpreadTable*>(gobj()));
}

Glib::PropertyProxy<guint> SpreadTable::property_vertical_spacing()
{
  return Glib::PropertyProxy<guint>(this, "vertical-spacing");
}

Glib::PropertyProxy_ReadOnly<guint> SpreadTable::property_vertical_spacing() const
{
  return Glib::PropertyProxy_ReadOnly<guint>(this, "vertical-spacing");
}

Glib::PropertyProxy<guint> SpreadTable::property_horizontal_spacing()
{
  return Glib::PropertyProxy<guint>(this, "horizontal-spacing");
}

Glib::PropertyProxy_ReadOnly<guint> SpreadTable::property_horizontal_spacing() const
{
  return Glib::PropertyProxy_ReadOnly<guint>(this, "horizontal-spacing");
}

Glib::PropertyProxy<guint> SpreadTable::property_lines()
{
  return Glib::PropertyProxy<guint>(this, "lines");
}

Glib::PropertyProxy_ReadOnly<guint> SpreadTable::property_lines() const
{
  return Glib::PropertyProxy_ReadOnly<guint>(this, "lines");
}

} // namespace Egg
