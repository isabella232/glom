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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_MODE_DESIGN_COMBO_SUMMARYTYPE_H
#define GLOM_MODE_DESIGN_COMBO_SUMMARYTYPE_H

#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <gtkmm/combobox.h>
#include <gtkmm/builder.h>

#include <gtkmm/liststore.h>

namespace Glom
{

class Combo_SummaryType : public Gtk::ComboBox
{
public:
  Combo_SummaryType(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Combo_SummaryType();

  void set_summary_type(LayoutItem_FieldSummary::summaryType summary_type);
  LayoutItem_FieldSummary::summaryType get_summary_type() const;

private:

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

} //namespace Glom

#endif //GLOM_MODE_DESIGN_COMBO_SUMMARYTYPE_H
