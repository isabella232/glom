/* Copyright (C) 2010 The gtkmm Development Team
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

#ifndef _EGG_SPREADTABLE_H
#define _EGG_SPREADTABLE_H

#include <gtkmm/container.h>
#include <gtkmm/orientable.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _EggSpreadTable EggSpreadTable;
typedef struct _EggSpreadTableClass EggSpreadTableClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Egg
{ class SpreadTable_Class; } // namespace Egg
namespace Egg
{

class SpreadTable
: public Gtk::Container,
  public Gtk::Orientable
{
  public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef SpreadTable CppObjectType;
  typedef SpreadTable_Class CppClassType;
  typedef EggSpreadTable BaseObjectType;
  typedef EggSpreadTableClass BaseClassType;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  virtual ~SpreadTable();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

private:
  friend class SpreadTable_Class;
  static CppClassType spreadtable_class_;

  // noncopyable
  SpreadTable(const SpreadTable&);
  SpreadTable& operator=(const SpreadTable&);

protected:
  explicit SpreadTable(const Glib::ConstructParams& construct_params);
  explicit SpreadTable(EggSpreadTable* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;


  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GtkObject.
  EggSpreadTable*       gobj()       { return reinterpret_cast<EggSpreadTable*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const EggSpreadTable* gobj() const { return reinterpret_cast<EggSpreadTable*>(gobject_); }


public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::


private:


public:
  SpreadTable();
  explicit SpreadTable(Gtk::Orientation orientation, guint lines);

  //TODO: Is the default packing appropriate (and like the default for a Box::pack_start())?

  //This is virtual to avoid us needing to override append_child() too in EggSpreadTableDnd
  virtual void insert_child(Gtk::Widget& widget, int index);


  guint get_child_line(const Gtk::Widget& child, int size) const;


  void set_lines(guint lines);

  guint get_lines() const;


  void set_vertical_spacing(guint spacing);

  guint get_vertical_spacing() const;


  void set_horizontal_spacing(guint spacing);

  guint get_horizontal_spacing() const;

  //TODO: Documentation
  void append_child(Widget& widget);

/** The amount of vertical space between two children.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<guint> property_vertical_spacing() ;

/** The amount of vertical space between two children.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<guint> property_vertical_spacing() const;

/** The amount of horizontal space between two children.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<guint> property_horizontal_spacing() ;

/** The amount of horizontal space between two children.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<guint> property_horizontal_spacing() const;

/** The number of lines (rows/columns) to evenly distribute children to.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<guint> property_lines() ;

/** The number of lines (rows/columns) to evenly distribute children to.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<guint> property_lines() const;
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
   * @relates Egg::SpreadTable
   */
  Egg::SpreadTable* wrap(EggSpreadTable* object, bool take_copy = false);
} //namespace Glib


#endif /* _EGG_SPREADTABLE_H */
