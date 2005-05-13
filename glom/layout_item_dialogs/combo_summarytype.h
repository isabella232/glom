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

#ifndef GLOM_MODE_DESIGN_COMBO_SUMMARYTYPE_HH
#define GLOM_MODE_DESIGN_COMBO_SUMMARYTYPE_HH

#include "../data_structure/layout/report_parts/layoutitem_fieldsummary.h"
#include <gtkmm/combobox.h>
#include <libglademm.h>

#include <gtkmm/liststore.h>


class Combo_SummaryType : public Gtk::ComboBox
{
public:
  Combo_SummaryType(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Combo_SummaryType();

  virtual void set_summary_type(LayoutItem_FieldSummary::summaryType summary_type);
  virtual LayoutItem_FieldSummary::summaryType get_summary_type() const;

protected:

  //Tree model columns:
  //These columns are used by the model that is created by the default constructor
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns()
    { add(m_summary_type); add(m_name); }

    Gtk::TreeModelColumn<LayoutItem_FieldSummary::summaryType> m_summary_type;
    Gtk::TreeModelColumn<Glib::ustring> m_name;
  };

  ModelColumns m_model_columns;
  Glib::RefPtr<Gtk::ListStore> m_model;
};

#endif //GLOM_MODE_DESIGN_COMBO_SUMMARYTYPE_HH

