/*
 *
 * Copyright 2011 The gtkmm Development Team
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

#include <glom/utility_widgets/eggspreadtablemm/eggspreadtabledndmm.h>
#include <glom/utility_widgets/eggspreadtablemm/private/eggspreadtabledndmm_p.h>


#include <glib.h>
#include <gtk/gtk.h>
#include <glom/utility_widgets/eggspreadtable/eggspreadtablednd.h>

namespace Glib
{

Egg::SpreadTableDnd* wrap(EggSpreadTableDnd* object, bool take_copy)
{
  return dynamic_cast<Egg::SpreadTableDnd *> (Glib::wrap_auto ((GObject*)(object), take_copy));
}

} /* namespace Glib */

namespace Egg
{


/* The *_Class implementation: */

const Glib::Class& SpreadTableDnd_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Class has to know the class init function to clone custom types.
    class_init_func_ = &SpreadTableDnd_Class::class_init_function;

    // This is actually just optimized away, apparently with no harm.
    // Make sure that the parent type has been created.
    //CppClassParent::CppObjectType::get_type();

    // Create the wrapper type, with the same class/instance size as the base type.
    register_derived_type(egg_spread_table_dnd_get_type());
  }

  return *this;
}


void SpreadTableDnd_Class::class_init_function(void* g_class, void* class_data)
{
  BaseClassType *const klass = static_cast<BaseClassType*>(g_class);
  CppClassParent::class_init_function(klass, class_data);


}


Glib::ObjectBase* SpreadTableDnd_Class::wrap_new(GObject* o)
{
  return manage(new SpreadTableDnd((EggSpreadTableDnd*)(o)));

}


/* The implementation: */

SpreadTableDnd::SpreadTableDnd(const Glib::ConstructParams& construct_params)
:
  SpreadTable(construct_params)
{
  }

SpreadTableDnd::SpreadTableDnd(EggSpreadTableDnd* castitem)
:
  SpreadTable((EggSpreadTable*)(castitem))
{
  }

SpreadTableDnd::~SpreadTableDnd()
{
  destroy_();
}

SpreadTableDnd::CppClassType SpreadTableDnd::spreadtable_class_; // initialize static member

GType SpreadTableDnd::get_type()
{
  return spreadtable_class_.init().get_type();
}


GType SpreadTableDnd::get_base_type()
{
  return egg_spread_table_dnd_get_type();
}


SpreadTableDnd::SpreadTableDnd()
:
  // Mark this class as non-derived to allow C++ vfuncs to be skipped.
  Glib::ObjectBase(0),
  SpreadTable(Glib::ConstructParams(spreadtable_class_.init()))
{


}

SpreadTableDnd::SpreadTableDnd(Gtk::Orientation orientation, guint lines)
:
  // Mark this class as non-derived to allow C++ vfuncs to be skipped.
  Glib::ObjectBase(0),
  SpreadTable(Glib::ConstructParams(spreadtable_class_.init(), "orientation", ((GtkOrientation)(orientation)), "lines", lines, static_cast<char*>(0)))
{
}

} // namespace Egg
