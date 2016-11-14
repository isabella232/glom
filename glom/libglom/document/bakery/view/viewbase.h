/*
 * Copyright 2000 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef GLOM_BAKERY_VIEWBASE_H
#define GLOM_BAKERY_VIEWBASE_H

#include <sigc++/trackable.h>

namespace GlomBakery
{

/** This is a base class for View.
 * This allows the App to call load_from_document() and save_to_document(),
 * without knowing exactly what type of document the view uses.
 */
class ViewBase : virtual public sigc::trackable
{
public:
  ViewBase();
  virtual ~ViewBase() = default;

  virtual void load_from_document();
  virtual void save_to_document();

  //Override these:
  virtual void clipboard_copy();
  virtual void clipboard_paste();
  virtual void clipboard_clear();
};

} //namespace

#endif
