/* Glom
 *
 * Copyright (C) 2010 Murray Cumming
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

#include <gtkmm.h>
#include <glom/utility_widgets/cellrendererlist.h>
#include <glom/mode_data/datawidget/cellrenderer_dblist.h>
#include <glom/mode_data/datawidget/cellrenderer_buttonimage.h>
#include <glom/mode_data/datawidget/cellrenderer_buttontext.h>
#include <glom/utils_ui.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libglom/data_structure/layout/layoutitem_image.h>
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <libglom/data_structure/glomconversions.h>


namespace Glom
{

static void apply_formatting(Gtk::CellRenderer* renderer, const sharedptr<const LayoutItem_WithFormatting>& layout_item)
{
  Gtk::CellRendererText* text_renderer = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!text_renderer)
    return;

  //Use the text formatting:

  //Horizontal alignment:
  const FieldFormatting::HorizontalAlignment alignment =
    layout_item->get_formatting_used_horizontal_alignment();
  const float x_align = (alignment == FieldFormatting::HORIZONTAL_ALIGNMENT_LEFT ? 0.0 : 1.0);
  text_renderer->property_xalign() = x_align;

  const FieldFormatting& formatting = layout_item->get_formatting_used();

  const Glib::ustring font_desc = formatting.get_text_format_font();
  if(!font_desc.empty())
  text_renderer->property_font() = font_desc;

  const Glib::ustring fg = formatting.get_text_format_color_foreground();
  if(!fg.empty())
    text_renderer->property_foreground() = fg;

  const Glib::ustring bg = formatting.get_text_format_color_background();
  if(!bg.empty())
    text_renderer->property_background() = bg;
}

Gtk::CellRenderer* create_cell(const sharedptr<const LayoutItem>& layout_item, const Glib::ustring& table_name, const Document* document, guint fixed_cell_height)
{
  Gtk::CellRenderer* cell = 0;

  //Create the appropriate cellrenderer type:
  sharedptr<const LayoutItem_Field> item_field = sharedptr<const LayoutItem_Field>::cast_dynamic(layout_item);
  if(item_field)
  {
    if(item_field->get_hidden())
      return 0;

    switch(item_field->get_glom_type())
    {
      case(Field::TYPE_BOOLEAN):
      {
         cell = Gtk::manage( new Gtk::CellRendererToggle() );

          break;
      }
      case(Field::TYPE_IMAGE):
      {
        cell = Gtk::manage( new Gtk::CellRendererPixbuf() );

        break;
      }
      default:
      {
        if(item_field->get_formatting_used().get_has_choices())
        {
          CellRendererDbList* rendererList = Gtk::manage( new CellRendererDbList() );
          sharedptr<LayoutItem> unconst = sharedptr<LayoutItem>::cast_const(layout_item); //TODO: Avoid this.
          rendererList->set_layout_item(unconst, table_name);
          bool as_radio_buttons = false; //Can't really be done in a list, so we ignore it.
          rendererList->set_restrict_values_to_list(
            item_field->get_formatting_used().get_choices_restricted(as_radio_buttons));

          cell = rendererList;
        }
        else
          cell = Gtk::manage( new Gtk::CellRendererText() );

        break;
      }
    }
  }
  else
  {
    //Non-fields:

    sharedptr<const LayoutItem_Image> item_image = sharedptr<const LayoutItem_Image>::cast_dynamic(layout_item);
    if(item_image)
    {
      Gtk::CellRendererPixbuf* pixbuf_renderer = Gtk::manage( new Gtk::CellRendererPixbuf() );

      const Glib::RefPtr<const Gdk::Pixbuf> pixbuf = Utils::get_pixbuf_for_gda_value(item_image->m_image);
      if(pixbuf)
        pixbuf_renderer->set_property("pixbuf", pixbuf);
      else
        pixbuf_renderer->set_property("stock-id", Gtk::StockID(Gtk::Stock::MISSING_IMAGE));

      cell = pixbuf_renderer;
    }
    else
    {
      sharedptr<const LayoutItem_Text> item_text = sharedptr<const LayoutItem_Text>::cast_dynamic(layout_item);
      if(item_text)
      {
        Gtk::CellRendererText* pCellText = Gtk::manage( new Gtk::CellRendererText() );
        pCellText->set_property("text", item_text->get_text());

        cell = pCellText;
      }
      else
      {
        sharedptr<const LayoutItem_Button> item_button = sharedptr<const LayoutItem_Button>::cast_dynamic(layout_item);
        if(item_button)
        {
          GlomCellRenderer_ButtonText* pCellButton = Gtk::manage( new GlomCellRenderer_ButtonText() );
          pCellButton->set_property("text", item_button->get_title_or_name());
          //pCellButton->set_fixed_width(50); //Otherwise it doesn't show up. TODO: Discover the width of the contents.

          cell = pCellButton;
        }
      }
    }
  }

  if(!cell)
    return 0;

  //Use formatting:
  sharedptr<const LayoutItem_WithFormatting> item_withformatting =
    sharedptr<const LayoutItem_WithFormatting>::cast_dynamic(layout_item);
  if(item_withformatting)
  {
    apply_formatting(cell, item_withformatting);
  }


  Gtk::CellRendererText* cell_text = dynamic_cast<Gtk::CellRendererText*>(cell);
  if(cell_text)
  {
    //Use an ellipze to indicate excessive text,
    //so that similar values do not look equal,
    //and to avoid multi-line comments. TODO: Is there a better way to restrict the height? This doesn't actually truncate multilines anyway.
    g_object_set(cell_text->gobj(), "ellipsize", PANGO_ELLIPSIZE_END, (gpointer)0);

    //Restrict the height, to prevent multiline text cells,
    //and to allow TreeView performance optimisation:
    //TODO: Avoid specifying a width for the last column?
    int suitable_width = 0;
    cell_text->get_property("width", suitable_width);
    cell_text->set_fixed_size(suitable_width, fixed_cell_height);
  }

  //Choices:
  CellRendererList* pCellRendererList = dynamic_cast<CellRendererList*>(cell);
  CellRendererDbList* pCellRendererDbList = dynamic_cast<CellRendererDbList*>(cell);
  if(pCellRendererList) //Used for custom choices:
  {
    pCellRendererList->remove_all_list_items();

    if(item_field && item_field->get_formatting_used().get_has_custom_choices())
    {
      //set_choices_fixed() needs this, for the numeric layout:
      //pCellRendererCombo->set_layout_item(get_layout_item()->clone(), table_name); //TODO_Performance: We only need this for the numerical format.
      const FieldFormatting::type_list_values list_values = item_field->get_formatting_used().get_choices_custom();
      for(FieldFormatting::type_list_values::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
      {
        pCellRendererList->append_list_item( Conversions::get_text_for_gda_value(item_field->get_glom_type(), *iter, item_field->get_formatting_used().m_numeric_format) );
      }
    }
  }
  else if(pCellRendererDbList) //Used for related choices:
  {
    if(item_field && item_field->get_formatting_used().get_has_related_choices())
    {
      sharedptr<const Relationship> choice_relationship;
      sharedptr<const LayoutItem_Field> choice_field;
      sharedptr<const LayoutGroup> choice_extras;
      bool choice_show_all = false;
      item_field->get_formatting_used().get_choices_related(choice_relationship, choice_field, choice_extras, choice_show_all);

      if(choice_relationship && choice_field)
      {
        const Glib::ustring to_table = choice_relationship->get_to_table();

        //TODO: Update this when the relationship's field value changes:
        if(choice_show_all) //Otherwise it must change whenever the relationships's ID value changes.
        {
          pCellRendererDbList->set_choices_related(document, item_field, Gnome::Gda::Value() /* TODO: Makes no sense */);
        }
      }
    }
  }

  return cell;
}

} //namespace Glom
