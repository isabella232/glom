/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
#include "flowtable.h"
#include "gtk/gtkwidget.h"
#include "gdk/gdktypes.h"

FlowTable::FlowTable()
: m_columns_count(1),
  m_padding(0)
{
  set_flags(Gtk::NO_WINDOW);
  set_redraw_on_allocate(false);
}

FlowTable::~FlowTable()
{
}

void FlowTable::add(Gtk::Widget& first, Gtk::Widget& second)
{
  FlowTableItem item;
  item.m_first = &first;
  item.m_second = &second;
  m_children.push_back(item);

  gtk_widget_set_parent(first.gobj(), GTK_WIDGET(gobj()));
  gtk_widget_set_parent(second.gobj(), GTK_WIDGET(gobj()));
}

void FlowTable::add(Gtk::Widget& first)
{
  type_base::add(first);
}

void FlowTable::set_columns_count(guint value)
{
  m_columns_count = value;
}

void FlowTable::get_item_requested_width(const FlowTableItem& item, guint& first, guint& second)
{
  //Initialize output paramters:
  first = 0;
  second = 0;
  
  Gtk::Widget* first_widget = item.m_first;
  Gtk::Widget* second_widget = item.m_second;
  guint result = 0;
  if(child_is_visible(first_widget) || child_is_visible(second_widget))
  {
    if(child_is_visible(first_widget))
    {
      //Discover how much space this child needs:
      Gtk::Requisition child_requisition_request;
      first_widget->size_request(child_requisition_request);  //TODO: This method should be const:
      first = child_requisition_request.width;
    }

    if(child_is_visible(second_widget))
    {
      //Discover how much space this child needs:
      Gtk::Requisition child_requisition_request;
      second_widget->size_request(child_requisition_request);  //TODO: This method should be const:
      second = child_requisition_request.width;
    }
  }
}

guint FlowTable::get_item_requested_height(const FlowTableItem& item)
{
  guint result = 0;
  
  Gtk::Widget* first = item.m_first;
  Gtk::Widget* second = item.m_second;
  guint max_child_height = 0;
  if(child_is_visible(first) || child_is_visible(second))
  {
    //Look at both widgets and see which one has most height:
    if(child_is_visible(first))
    {
      // Ask the child how much space it needs:
      Gtk::Requisition child_requisition;
      gtk_widget_size_request(const_cast<GtkWidget*>(first->gobj()), &child_requisition); //TODO: This parameter should not be const: child->size_request(child_requisition);
      max_child_height = child_requisition.height;
    }

    if(child_is_visible(second))
    {
      // Ask the child how much space it needs:
      Gtk::Requisition child_requisition;
      gtk_widget_size_request(const_cast<GtkWidget*>(second->gobj()), &child_requisition); //TODO: This parameter should not be const: child->size_request(child_requisition);
      max_child_height = MAX(max_child_height, child_requisition.height);
    }

    //Use the largest height (they are next to each other, horizontally):
    result += max_child_height;
  }

  return result;
}

guint FlowTable::get_column_height(guint start_widget, guint widget_count, guint& total_width)
{
    //initialize output parameter:
    total_width = 0;
  
    //Just add the heights together:
    guint column_height = 0;
    guint column_width_first = 0;
    guint column_width_second = 0;
    guint i = 0;
    for(i = start_widget; i < (start_widget+widget_count);  ++i)
    {
      FlowTableItem item = m_children[i];
      guint item_height = get_item_requested_height(item);
      
      guint item_width_first = 0;
      guint item_width_second = 0;
      get_item_requested_width(item, item_width_first, item_width_second);

      column_height += item_height;

      //Add the padding if it's not the first widget, and if one is visible:
      if( (i != start_widget) && item_height)
      {
        column_height += m_padding;
      }

      if(item_height) //If one was visible.
      {
        column_width_first =MAX(column_width_first, item_width_first);
        column_width_second =MAX(column_width_second, item_width_second);
      }

    }

     total_width = column_width_first +  column_width_second;
     if( (column_width_first > 0) && (column_width_second > 0) ) //Add padding if necessary.
       total_width += m_padding;
       
    return column_height;
}  

guint FlowTable::get_minimum_column_height(guint start_widget, guint columns_count, guint& total_width)
{  
  //initialize output parameter:
  total_width = 0;
  
  //Discover how best (least column height) to arrange these widgets in these columns, keeping them in sequence,
  //and then say how high the columns must best.
  
  if(columns_count == 1)
  {
    //Just add the heights together:
    const guint widgets_count = m_children.size() - start_widget;
    guint column_height = get_column_height(start_widget, widgets_count, total_width);

    return column_height;
  }
  else
  {
    //Try each combination of widgets in the first column, combined with the the other combinations in the following columns:
    guint minimum_column_height = 0;
    guint width_for_minimum_column_height = 0;
    bool at_least_one_combination_checked = false;

    const guint count_items_remaining = m_children.size() - start_widget;
    
    for(guint first_column_widgets_count = 1;  first_column_widgets_count < count_items_remaining; ++first_column_widgets_count)
    {
      guint first_column_width = 0;
      guint first_column_height = get_column_height(start_widget, first_column_widgets_count, first_column_width);
      guint minimum_column_height_sofar = first_column_height;
        
      guint others_column_width = 0;
      const guint others_column_start_widget =  start_widget + first_column_widgets_count;
      //Call this function recursively to get the minimum column height in the other columns, when these widgets are in the first column:
      guint minimum_column_height_nextcolumns = 0;
      if( others_column_start_widget  < m_children.size())
      {
        minimum_column_height_nextcolumns = get_minimum_column_height(others_column_start_widget, columns_count - 1, others_column_width);
        others_column_width += m_padding; //Add the padding between the previous column and this one.

        minimum_column_height_sofar = MAX(first_column_height, minimum_column_height_nextcolumns);
      }

      //See whether this is better than the last one:
      if(at_least_one_combination_checked)
      {
        if(minimum_column_height_sofar < minimum_column_height)
        {
          minimum_column_height = minimum_column_height_sofar;
          width_for_minimum_column_height = first_column_width + others_column_width; 
        }
      }
      else
      {
        minimum_column_height = minimum_column_height_sofar;
        width_for_minimum_column_height = first_column_width + others_column_width;

        at_least_one_combination_checked = true;
      }
    }

    total_width = width_for_minimum_column_height;

    return minimum_column_height;
  }
}

void FlowTable::on_size_request(Gtk::Requisition* requisition)
{
  //Initialize the output parameter:
  *requisition = Gtk::Requisition();

  //Discover the total amount of minimum space needed by this container widget, by examining its child widgets,
  //by examing every possible sequential arrangement of the widgets in this fixed number of columsn:
  guint total_width = 0;
  const guint column_height = get_minimum_column_height(0, m_columns_count, total_width); //This calls itself recursively.
  
  requisition->height = column_height;
  requisition->width = total_width;
}


//Give it whatever height/width it wants, at this location:
void FlowTable::assign_child(Gtk::Widget* widget, guint x, guint y)
{
  //Discover how much space this child needs:
  Gtk::Requisition child_requisition_request;
  widget->size_request(child_requisition_request);  //TODO: This method should be const:

  //Assign sign space to the child:
  Gtk::Allocation child_allocation;

  child_allocation.set_x(x);
  child_allocation.set_y(y);

  //Give it as much space as it wants:
  child_allocation.set_width(child_requisition_request.width);
  child_allocation.set_height(child_requisition_request.height);

  //g_warning("  assigning: x=%d, y=%d, width=%d, height=%d", x, y, child_requisition_request.width, child_requisition_request.height);
  //g_warning("    far x=%d, far y=%d", x+child_requisition_request.width , y+child_requisition_request.height);

  widget->size_allocate(child_allocation);
}

void FlowTable::get_item_max_width(guint start, guint height, guint& first_max_width, guint& second_max_width)
{
  //Initialize output parameters:
  first_max_width = 0;
  second_max_width = 0;
  
  if(m_children.size() == 0)
    return;

  guint height_so_far = 0;
  bool first_item_added = false;
  guint i = start;
  while( (height_so_far < height) && (i < m_children.size()))
  {
    FlowTableItem item = m_children[i];
    Gtk::Widget* first = item.m_first;
    Gtk::Widget* second = item.m_second;

    guint height_item = 0;

    //Add padding above the item, if it is after another one.
    if( first_item_added && (child_is_visible(first) || child_is_visible(second)) )
    {
       height_item += m_padding;
    }

    guint item_first_width = 0;
    if(child_is_visible(first))
    {
      Gtk::Requisition child_requisition;
      gtk_widget_size_request(const_cast<GtkWidget*>(first->gobj()), &child_requisition); //TODO: This parameter should not be const: child->size_request(child_requisition);     
      item_first_width = child_requisition.width;
      height_item = MAX(height_item, child_requisition.height);
      first_item_added = true;
    }

    guint item_second_width = 0;
    if(child_is_visible(second))
    {
      Gtk::Requisition child_requisition;
      gtk_widget_size_request(const_cast<GtkWidget*>(second->gobj()), &child_requisition); //TODO: This parameter should not be const: child->size_request(child_requisition);
      item_second_width = child_requisition.width;
      height_item = MAX(height_item, child_requisition.height);
      first_item_added = true;                                       
    }

    height_so_far += height_item;
    if(height_so_far <= height) //Don't remember the width details if the widgets are too high for the end of this column:
    {
      first_max_width = MAX(first_max_width, item_first_width);
      second_max_width = MAX(second_max_width, item_second_width);
    }

    ++i;
  }

  //g_warning("i=%d", i);
  //g_warning("get_item_max_width(guint start=%d, guint height=%d, guint& first_max_width=%d, guint& second_max_width=%d",  start, height, first_max_width, second_max_width);
}

void FlowTable::on_size_allocate(Gtk::Allocation& allocation)
{
  //Do something with the space that we have actually been given:
  //(We will not be given heights or widths less than we have requested, though we might get more)
      
  set_allocation(allocation);

  //Discover the widths of the different parts of the first column:
  guint first_max_width = 0;
  guint second_max_width= 0;
  get_item_max_width(0, allocation.get_height(), first_max_width, second_max_width);  //TODO: Give the 2nd part of the column a bit more if the total needed is less than the allocation given.
  //Calculate where the columns should start on the x axis.
  guint column_x_start = allocation.get_x();
  
  guint column_x_start_second = column_x_start + first_max_width;
  if(first_max_width > 0) //Add padding between first and second sub sets of items, if there is a first set.
    column_x_start_second += m_padding;

  guint column_child_y_start = allocation.get_y();

  bool first_vertical_padding_added = false;

  guint count = m_children.size();
  for(guint i = 0; i < count; ++i)
  {
    FlowTableItem item = m_children[i];

    guint item_height = get_item_requested_height(item);

  
   
    //Start a new column if necessary:
    guint bottom = allocation.get_y() + allocation.get_height();   
    if( (column_child_y_start + item_height) > bottom)
    {
      //start a new column:
      column_child_y_start = allocation.get_y();
      column_x_start += column_x_start_second + second_max_width + m_padding;

      {
        //Discover the widths of the different parts of this column:
        guint first_max_width = 0;
        guint second_max_width= 0;
        get_item_max_width(i, allocation.get_height(), first_max_width, second_max_width);

        column_x_start_second = column_x_start + first_max_width;
        if(first_max_width > 0) //Add padding between first and second sub sets of items, if there is a first set.
          column_x_start_second += m_padding;
      }
    }

    bool something_added = false;
    
    Gtk::Widget* first = item.m_first;
    if(child_is_visible(first))
    {
      //Assign sign space to the child:
      
      //Make all the left edges line up:
      assign_child(first, column_x_start, column_child_y_start);
      something_added = true;
    }

    Gtk::Widget* second = item.m_second;
    if(child_is_visible(second))
    {
      //Assign sign space to the child:

      //Make all the left edges line up:
      assign_child(second, column_x_start_second, column_child_y_start);
      something_added = true;
    }

    if(something_added)
    {
      //Start the next child below this child, plus the padding
      column_child_y_start += item_height;
      column_child_y_start += m_padding; //Ignored if this is the last item - we will just start a new column when we find that column_child_y_start is too much.
    }
  }
}

GtkType FlowTable::child_type_vfunc() const
{
  //TODO: What is this for?
  if(!m_children.empty())
    return GTK_TYPE_WIDGET;
  else
    return G_TYPE_NONE;
}


void FlowTable::on_add(Gtk::Widget* child)
{
  FlowTableItem item;
  item.m_first = child;
  item.m_second = 0;
  m_children.push_back(item);
  
  gtk_widget_set_parent(child->gobj(), GTK_WIDGET(gobj()));

  //This is protected, but should be public: child.set_parent(*this);

}


void FlowTable::on_remove(Gtk::Widget* child)
{
  //TODO:
  /*
  type_vecChildren::iterator iterFind = std::find(m_children.begin(), m_children.end(), child);
  if(iterFind != m_children.end())
  {
    m_children.erase(iterFind);

    gtk_widget_unparent(child->gobj());

    //This is protected, but should be public: child.unparent();
  }
  */
}


void FlowTable::forall_vfunc(gboolean include_internals, GtkCallback callback, gpointer callback_data)
{
  for(type_vecChildren::const_iterator iter = m_children.begin(); iter != m_children.end(); ++iter)
  {
    FlowTableItem item = *iter;
    
    Gtk::Widget* first = item.m_first;
    if(first)
      callback(first->gobj(), callback_data);

    Gtk::Widget* second = item.m_second;
    if(second)
      callback(second->gobj(), callback_data);
  }
}

/** Sets the padding to put between the child widgets.
 */
void FlowTable::set_padding(guint padding)
{
  m_padding = padding; 
}

bool FlowTable::child_is_visible(Gtk::Widget* widget)
{
  return widget && widget->is_visible();
}

void FlowTable::remove_all()
{
  for(type_vecChildren::iterator iter = m_children.begin(); iter != m_children.end(); ++iter)
  {
    if(iter->m_first)
      gtk_widget_unparent(iter->m_first->gobj());

    if(iter->m_second)
      gtk_widget_unparent(iter->m_second->gobj());
  }
   
  m_children.clear(); 
}
