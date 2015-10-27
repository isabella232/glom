/* Copyright (C) 2011 The gtkmm Development Team
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

#ifndef _EGG_SPREADTABLE_DND_H
#define _EGG_SPREADTABLE_DND_H

#include <glom/utility_widgets/eggspreadtablemm/eggspreadtablemm.h>
#include <glom/utility_widgets/eggspreadtable/eggspreadtablednd.h> //For the enum, though we could wrap it instead.


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _EggSpreadTableDnd EggSpreadTableDnd;
typedef struct _EggSpreadTableDndClass EggSpreadTableDndClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Egg
{ class SpreadTableDnd_Class; } // namespace Egg
namespace Egg
{

class SpreadTableDnd
: public SpreadTable
{
  public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef SpreadTableDnd CppObjectType;
  typedef SpreadTableDnd_Class CppClassType;
  typedef EggSpreadTableDnd BaseObjectType;
  typedef EggSpreadTableDndClass BaseClassType;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  virtual ~SpreadTableDnd();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

private:
  friend class SpreadTableDnd_Class;
  static CppClassType spreadtable_class_;

  // noncopyable
  SpreadTableDnd(const SpreadTableDnd&);
  SpreadTableDnd& operator=(const SpreadTableDnd&);

protected:
  explicit SpreadTableDnd(const Glib::ConstructParams& construct_params);
  explicit SpreadTableDnd(EggSpreadTableDnd* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;


  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GtkObject.
  EggSpreadTableDnd*       gobj()       { return reinterpret_cast<EggSpreadTableDnd*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const EggSpreadTableDnd* gobj() const { return reinterpret_cast<EggSpreadTableDnd*>(gobject_); }


public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::

  bool on_widget_drop_possible(Gtk::Widget* widget, bool& drop_possible);

private:


public:
  SpreadTableDnd();
  explicit SpreadTableDnd(Gtk::Orientation orientation, guint lines);

  void insert_child(Gtk::Widget& child, int index) override;
  void remove_child(Gtk::Widget& child);
  void set_drag_enabled(EggDragEnableMode drag_enabled);
  EggDragEnableMode get_drag_enabled() const;
  void set_drop_enabled(bool drop_enabled = true);
  bool get_drop_enabled() const;


  /**
   * @par Prototype:
   * <tt>void on_my_%widget_drop_possible()</tt>
   */

  Glib::SignalProxy2< bool, Gtk::Widget*, bool& > signal_widget_drop_possible();
};

} // namespace Egg


namespace Glib
{
  /** A Glib::wrap() method for this object.
   *
   * @param object The C instance.
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   *
   * @relates Egg::SpreadTableDnd
   */
  Egg::SpreadTableDnd* wrap(EggSpreadTableDnd* object, bool take_copy = false);
} //namespace Glib


#endif /* _EGG_SPREADTABLE_H */
