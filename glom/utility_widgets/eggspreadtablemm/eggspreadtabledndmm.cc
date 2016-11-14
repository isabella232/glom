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
#include <glibmm/exceptionhandler.h>

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

namespace
{

static gboolean EggSpreadTableDnd_signal_widget_drop_possible_callback(EggSpreadTableDnd* self, GtkWidget* p0, gboolean* drop_possible, void* data)
{
  using namespace Egg;
  typedef sigc::slot< bool, Gtk::Widget*, bool& > SlotType;

  // Do not try to call a signal on a disassociated wrapper.
  if(Glib::ObjectBase::_get_current_wrapper((GObject*) self))
  {
    try
    {
      if(sigc::slot_base *const slot = Glib::SignalProxyNormal::data_to_slot(data))
      {
        bool cpp_drop_possible = false;
        const auto result = static_cast<int>((*static_cast<SlotType*>(slot))(Glib::wrap(p0), cpp_drop_possible));
        *drop_possible = cpp_drop_possible;
        return result;
      }
    }
    catch(...)
    {
      Glib::exception_handlers_invoke();
    }
  }

  using RType = gboolean;
  return RType();
}

static gboolean EggSpreadTableDnd_signal_widget_drop_possible_notify_callback(EggSpreadTableDnd* self, GtkWidget* p0, gboolean* drop_possible, void* data)
{
  using namespace Egg;
  typedef sigc::slot< bool, Gtk::Widget*, bool& > SlotType;

  // Do not try to call a signal on a disassociated wrapper.
  if(Glib::ObjectBase::_get_current_wrapper((GObject*) self))
  {
    try
    {
      if(sigc::slot_base *const slot = Glib::SignalProxyNormal::data_to_slot(data))
      {
        bool cpp_drop_possible = false;
        const auto result = static_cast<int>((*static_cast<SlotType*>(slot))(Glib::wrap(p0), cpp_drop_possible));
        *drop_possible = cpp_drop_possible;
        return result;
      }
    }
    catch(...)
    {
      Glib::exception_handlers_invoke();
    }
  }

  using RType = gboolean;
  return RType();
}

static const Glib::SignalProxyInfo SpreadTableDnd_signal_widget_drop_possible_info =
{
  "widget-drop-possible",
  (GCallback) &EggSpreadTableDnd_signal_widget_drop_possible_callback,
  (GCallback) &EggSpreadTableDnd_signal_widget_drop_possible_notify_callback
};


} //anonymous namespace


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

  klass->widget_drop_possible = &widget_drop_possible_callback;
}

gboolean SpreadTableDnd_Class::widget_drop_possible_callback(EggSpreadTableDnd* self, GtkWidget* p0, gboolean* drop_possible)
{
  Glib::ObjectBase *const obj_base = static_cast<Glib::ObjectBase*>(
      Glib::ObjectBase::_get_current_wrapper((GObject*)self));

  // Non-gtkmmproc-generated custom classes implicitly call the default
  // Glib::ObjectBase constructor, which sets is_derived_. But gtkmmproc-
  // generated classes can use this optimisation, which avoids the unnecessary
  // parameter conversions if there is no possibility of the virtual function
  // being overridden:
  if(obj_base && obj_base->is_derived_())
  {
    CppObjectType *const obj = dynamic_cast<CppObjectType* const>(obj_base);
    if(obj) // This can be NULL during destruction.
    {
      try // Trap C++ exceptions which would normally be lost because this is a C callback.
      {
        // Call the virtual member method, which derived classes might override.
        bool cpp_drop_possible = false;
        const bool result =
          static_cast<int>(obj->on_widget_drop_possible(Glib::wrap(p0),
            cpp_drop_possible));
        *drop_possible = cpp_drop_possible;
        return result;
      }
      catch(...)
      {
        Glib::exception_handlers_invoke();
      }
    }
  }

  BaseClassType *const base = static_cast<BaseClassType*>(
        g_type_class_peek_parent(G_OBJECT_GET_CLASS(self)) // Get the parent class of the object class (The original underlying C class).
    );

  // Call the original underlying C function:
  if(base && base->widget_drop_possible)
    return (*base->widget_drop_possible)(self, p0, drop_possible);

  using RType = gboolean;
  return RType();
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
  Glib::ObjectBase(nullptr),
  SpreadTable(Glib::ConstructParams(spreadtable_class_.init()))
{
}

SpreadTableDnd::SpreadTableDnd(Gtk::Orientation orientation, guint lines)
:
  // Mark this class as non-derived to allow C++ vfuncs to be skipped.
  Glib::ObjectBase(nullptr),
  SpreadTable(Glib::ConstructParams(spreadtable_class_.init(), "orientation", ((GtkOrientation)(orientation)), "lines", lines, static_cast<char*>(nullptr)))
{
}

void SpreadTableDnd::insert_child(Gtk::Widget& child, int index)
{
  egg_spread_table_dnd_insert_child(gobj(), child.gobj(), index);
}

void SpreadTableDnd::remove_child(Gtk::Widget& child)
{
  //This is based on Gtk::Container::remove()
  //We don't need to do this often, because specialized remove() functions are unusual:
  //
  //If this is a managed widget,
  //then do an extra ref so that it will
  //not be destroyed when adding to another container
  //This should leave it in much the same state as when it was instantiated,
  //before being added to the first container.
  if(child.is_managed_())
    child.reference();

  egg_spread_table_dnd_remove_child(gobj(), child.gobj());
}

void SpreadTableDnd::set_drag_enabled(EggDragEnableMode drag_enabled)
{
  egg_spread_table_dnd_set_drag_enabled(gobj(), drag_enabled);
}

EggDragEnableMode SpreadTableDnd::get_drag_enabled() const
{
  return egg_spread_table_dnd_get_drag_enabled(const_cast<EggSpreadTableDnd*>(gobj()));
}

void SpreadTableDnd::set_drop_enabled(bool drop_enabled)
{
  egg_spread_table_dnd_set_drop_enabled(gobj(), drop_enabled);
}

bool SpreadTableDnd::get_drop_enabled() const
{
  return egg_spread_table_dnd_get_drop_enabled(const_cast<EggSpreadTableDnd*>(gobj()));
}

bool SpreadTableDnd::on_widget_drop_possible(Gtk::Widget* widget, bool& drop_possible)
{
  BaseClassType *const base = static_cast<BaseClassType*>(
      g_type_class_peek_parent(G_OBJECT_GET_CLASS(gobject_)) // Get the parent class of the object class (The original underlying C class).
  );

  if(base && base->widget_drop_possible)
  {
    gboolean c_drop_possible = FALSE;
    const gboolean result =
      (*base->widget_drop_possible)(gobj(), Glib::unwrap(widget), &c_drop_possible);
    drop_possible = c_drop_possible;
    return result;
  }
  else
    return false;
}

Glib::SignalProxy2< bool, Gtk::Widget*, bool& > SpreadTableDnd::signal_widget_drop_possible()
{
  return Glib::SignalProxy2< bool, Gtk::Widget*, bool& >(this, &SpreadTableDnd_signal_widget_drop_possible_info);
}

} // namespace Egg
