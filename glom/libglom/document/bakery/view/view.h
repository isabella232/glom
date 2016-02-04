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

#ifndef GLOM_BAKERY_VIEW_H
#define GLOM_BAKERY_VIEW_H

#include <libglom/document/bakery/view/viewbase.h>
#include <libglom/document/bakery/document.h>
#include <sigc++/sigc++.h>
#include <memory>

namespace GlomBakery
{

/** This is a base class which should be multiple-inherited with gtkmm widgets.
  * You should override save_to_document() and load_from_document().
  */
template< class T_Document >
class View : public ViewBase
{
public: 
  View()
  {
  }

  typedef View<T_Document> type_self;

  //typedef typename T_Document type_document;

  virtual std::shared_ptr<T_Document> get_document()
  {
    return m_document;
  }

  virtual std::shared_ptr<const T_Document> get_document() const
  {
    return m_document;
  }

  virtual void set_document(const std::shared_ptr<T_Document>& document)
  {
    m_document = document;
  }

  ///Just a convenience, instead of get_docuement()->set_modified().
  virtual void set_modified(bool val = true)
  {
    if(m_document)
      m_document->set_modified(val);
  }

protected:
  
  std::shared_ptr<T_Document> m_document;
};

} //namespace

#endif //BAKERY_VIEW_H

