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

#ifndef GLOM_BAKERY_VIEW_COMPOSITE_H
#define GLOM_BAKERY_VIEW_COMPOSITE_H

#include <libglom/document/bakery/view/view.h>
#include <libglom/algorithms_utils.h>
#include <vector>
#include <algorithm> //For std::find

namespace GlomBakery
{

/** This View delegates to sub-views.
 * It is very simplistic - maybe your View should be more intelligent.
 */
template< class T_Document >
class View_Composite : public View<T_Document>
{
public: 
  View_Composite()
  {
  }

  typedef View<T_Document> type_view;

  virtual void add_view(type_view* pView)
  {
    //Ensure that the view has the same document:
    //This should be unnecessary.
    if(pView)
    {
      pView->set_document(View<T_Document>::get_document());

      //Add it to the list of child views:
      m_vecViews.push_back(pView);
    }
  }

  virtual void remove_view(type_view* pView)
  {
    auto iter = Glom::Utils::find(m_vecViews, pView);
    if(iter != m_vecViews.end())
      m_vecViews.erase(iter);
  }

  void set_document(T_Document* pDocument) override
  {
    //Call base class:
    View<T_Document>::set_document(pDocument);

    //Change the document in the child views.
    for(const auto& pView : m_vecViews)
    {
      if(pView)
        pView->set_document(pDocument);
    }
  }

  void load_from_document() override
  {
    //Delegate to the child views:
    for(const auto& pView : m_vecViews)
    {
      if(pView)
        pView->load_from_document();
    }
  }

  void save_to_document() override
  {
    //Delegate to the child views:
    for(const auto& pView : m_vecViews)
    {
      if(pView)
        pView->save_to_document();
    }
  }

protected:
  typedef std::vector<type_view*> type_vec_views;
  type_vec_views m_vecViews;    
};

} //namespace

#endif
