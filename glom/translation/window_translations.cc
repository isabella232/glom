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

#include "window_translations.h"
#include "combobox_locale.h"
#include "dialog_identify_original.h"
#include "dialog_copy_translation.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include "glom/libglom/utils.h"

#include <glibmm/i18n.h>

// To read the .po files
#include <gettext-po.h>
#include "config.h" //For HAVE_GETTEXTPO_XERROR

#include <libgnomevfsmm/handle.h>
#include <sstream>

namespace Glom
{

Window_Translations::Window_Translations(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Window(cobject),
  m_treeview(0),
  m_button_identify(0),
  m_combo_target_locale(0),
  m_label_source_locale(0),
  m_button_ok(0),
  m_button_cancel(0),
  m_button_import(0),
  m_button_export(0),
  m_treeview_modified(false)
{
  refGlade->get_widget("label_source_locale", m_label_source_locale);

  refGlade->get_widget("treeview", m_treeview);
  if(m_treeview)
  {
    m_model = Gtk::ListStore::create(m_columns);
    m_treeview->set_model(m_model);

    // Append the View columns:
    Gtk::TreeView::Column* column_original = Gtk::manage( new Gtk::TreeView::Column(_("Original")) );
    m_treeview->append_column(*column_original);

    Gtk::CellRendererText* renderer_name = Gtk::manage(new Gtk::CellRendererText);
    column_original->pack_start(*renderer_name);
    column_original->set_cell_data_func(*renderer_name, sigc::mem_fun(*this, &Window_Translations::on_cell_data_original));


    Gtk::TreeView::Column* column_item_typename = Gtk::manage( new Gtk::TreeView::Column(_("Item")) );
    m_treeview->append_column(*column_item_typename);

    Gtk::CellRendererText* renderer_item_typename = Gtk::manage(new Gtk::CellRendererText);
    column_item_typename->pack_start(*renderer_item_typename);
    column_item_typename->set_cell_data_func(*renderer_item_typename, sigc::mem_fun(*this, &Window_Translations::on_cell_data_item_typename));


    const int col = m_treeview->append_column_editable(_("Translation"), m_columns.m_col_translation);
    Gtk::CellRendererText* renderer = dynamic_cast<Gtk::CellRendererText*>(m_treeview->get_column_cell_renderer(col - 1));
    if(renderer)
      renderer->signal_edited().connect(sigc::mem_fun(*this, &Window_Translations::on_treeview_edited));
  }

  refGlade->get_widget("button_identify", m_button_identify);
  m_button_identify->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_identify) );

  refGlade->get_widget_derived("combobox_target_locale", m_combo_target_locale);
  m_combo_target_locale->signal_changed().connect(sigc::mem_fun(*this, &Window_Translations::on_combo_target_locale_changed));

  refGlade->get_widget("button_ok", m_button_ok);
  m_button_ok->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_ok) );

  refGlade->get_widget("button_cancel", m_button_cancel);
  m_button_cancel->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_cancel) );

  refGlade->get_widget("button_copy_translation", m_button_copy_translation);
  m_button_copy_translation->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_copy_translation) );

  refGlade->get_widget("button_import", m_button_import);
  m_button_import->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_import) );

  refGlade->get_widget("button_export", m_button_export);
  m_button_export->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_export) );

  show_all_children();

  //Start with the currently-used/tested translation, if appropriate:
  if(TranslatableItem::get_current_locale_not_original())
  {
    m_translation_locale = TranslatableItem::get_current_locale();
    m_combo_target_locale->set_selected_locale(m_translation_locale);
    //The translations will be shown in the treeview when load_from_document() is called.
  }
}

Window_Translations::~Window_Translations()
{
}


void Window_Translations::enable_buttons()
{

}

void Window_Translations::on_button_identify()
{
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom_developer.glade", "dialog_translation_identify_original");
  if(refXml)
  {
    Dialog_IdentifyOriginal* dialog = 0;
    refXml->get_widget_derived("dialog_translation_identify_original", dialog);
    if(dialog)
    {
      add_view(dialog);
      dialog->load_from_document(); //Doesn't seem to happen otherwise.
      dialog->set_transient_for(*this);
      const int response = Glom::Utils::dialog_run_with_help(dialog, "dialog_translation_identify_original");
      dialog->hide();

      if(response == Gtk::RESPONSE_OK)
      {
        get_document()->set_translation_original_locale(dialog->get_locale());

        //Save and update:
        on_combo_target_locale_changed();
      }

      remove_view(dialog);
      delete dialog;
    }
  }
}


void Window_Translations::on_cell_data_original(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      Glib::ustring text;
      sharedptr<TranslatableItem> item = row[m_columns.m_col_item];
      if(item)
        text = item->get_title_original();

      //Use the name if there is no title:
      if(text.empty())
        text = item->get_name(); 

      //TODO: Mark non-English originals.
      renderer_text->property_text() = text;
      renderer_text->property_editable() = false; //Names can never be edited.
    }
  }
}

void Window_Translations::on_cell_data_item_typename(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      Glib::ustring item_type_name;
      sharedptr<TranslatableItem> item = row[m_columns.m_col_item];
      if(item)
        item_type_name = TranslatableItem::get_translatable_type_name(item->get_translatable_item_type());

      renderer_text->property_text() = item_type_name;
      renderer_text->property_editable() = false; //Names can never be edited.
    }
  }
}

void Window_Translations::load_from_document()
{
  m_model->clear(); //Remove all rows.

  Document_Glom* document = get_document();
  if(!document)
    return;

  //std::cout << "document->get_translation_original_locale()=" << document->get_translation_original_locale() << std::endl;
  Glib::ustring original_locale_name = IsoCodes::get_locale_name(document->get_translation_original_locale());
  if(original_locale_name.empty())
    original_locale_name = _("Unknown");
  m_label_source_locale->set_text(original_locale_name);

  //Add tables:
  Document_Glom::type_listTableInfo tables = document->get_tables();
  for(Document_Glom::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    sharedptr<TableInfo> tableinfo = *iter;
    const Glib::ustring table_name = tableinfo->get_name();

    //Table title:
    Gtk::TreeModel::iterator iterTree = m_model->append();
    Gtk::TreeModel::Row row = *iterTree;
    row[m_columns.m_col_item] = tableinfo;
    row[m_columns.m_col_translation] = tableinfo->get_title(m_translation_locale);
    row[m_columns.m_col_parent_table] = Glib::ustring(); //Not used for tables.

    //The table's field titles:
    Document_Glom::type_vecFields fields = document->get_table_fields(table_name);
    for(Document_Glom::type_vecFields::iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterTree = m_model->append();
      Gtk::TreeModel::Row row = *iterTree;

      sharedptr<Field> field = *iter;
      row[m_columns.m_col_item] = field;
      row[m_columns.m_col_translation] = field->get_title(m_translation_locale);
      row[m_columns.m_col_parent_table] = table_name;

    }

    //The table's relationships:
    Document_Glom::type_vecRelationships relationships = document->get_relationships(table_name);
    for(Document_Glom::type_vecRelationships::iterator iter = relationships.begin(); iter != relationships.end(); ++iter)
    {
      sharedptr<Relationship> relationship = *iter;
      if(relationship)
      {
        Gtk::TreeModel::iterator iterTree = m_model->append();
        Gtk::TreeModel::Row row = *iterTree;

        row[m_columns.m_col_item] = relationship;
        row[m_columns.m_col_translation] = relationship->get_title(m_translation_locale);
        row[m_columns.m_col_parent_table] = table_name;
      }
    }

    //The table's report titles:
    Document_Glom::type_listReports listReports = document->get_report_names(table_name);
    for(Document_Glom::type_listReports::iterator iter = listReports.begin(); iter != listReports.end(); ++iter)
    {
      const Glib::ustring report_name = *iter;
      sharedptr<Report> report = document->get_report(table_name, report_name);
      if(report)
      {
        //Translatable report title:
        Gtk::TreeModel::iterator iterTree = m_model->append();
        Gtk::TreeModel::Row row = *iterTree;

        row[m_columns.m_col_item] = report;
        row[m_columns.m_col_translation] = report->get_title(m_translation_locale);
        row[m_columns.m_col_parent_table] = table_name;

        //Translatable report items:
        Document_Glom::type_list_translatables list_layout_items = document->get_translatable_report_items(table_name, report_name);
        for(Document_Glom::type_list_translatables::iterator iter = list_layout_items.begin(); iter != list_layout_items.end(); ++iter)
        {
          sharedptr<TranslatableItem> item = *iter;
          if(item)
          {
            if(!(item->get_title_original().empty()))
            {
              Gtk::TreeModel::iterator iterTree = m_model->append();
              Gtk::TreeModel::Row row = *iterTree;

              row[m_columns.m_col_item] = item;
              row[m_columns.m_col_translation] = item->get_title(m_translation_locale);
              row[m_columns.m_col_parent_table] = table_name;
            }
          }
        }
      }
    }

    //The table's translatable layout items:
    Document_Glom::type_list_translatables list_layout_items = document->get_translatable_layout_items(table_name);
    for(Document_Glom::type_list_translatables::iterator iter = list_layout_items.begin(); iter != list_layout_items.end(); ++iter)
    {
      sharedptr<TranslatableItem> item = *iter;
      if(item)
      {
        if(!(item->get_title_original().empty()))
        {
          Gtk::TreeModel::iterator iterTree = m_model->append();
          Gtk::TreeModel::Row row = *iterTree;

          row[m_columns.m_col_item] = item;
          row[m_columns.m_col_translation] = item->get_title(m_translation_locale);
          row[m_columns.m_col_parent_table] = table_name;
        }
      }
    }

  } //for

  m_treeview_modified = false;
}

void Window_Translations::save_to_document()
{
  if(!m_treeview_modified || m_translation_locale.empty())
    return;

  //Look at every item in the treeview and apply its translation:
  for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
  {
    Gtk::TreeModel::Row row = *iter;

    //We have stored a sharedptr to the original item, so we can just change it directly:
    sharedptr<TranslatableItem> item = row[m_columns.m_col_item];
    if(item)
    {
      const Glib::ustring translation = row[m_columns.m_col_translation];
      item->set_title(m_translation_locale, translation);
    }
  }

  m_treeview_modified = false;
  set_modified(); //Save to the document.
}

void Window_Translations::on_button_cancel()
{
  hide();
}

void Window_Translations::on_button_ok()
{
  save_to_document();
  hide();
}

void Window_Translations::on_button_copy_translation()
{
   Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom_developer.glade", "dialog_translation_copy");
  if(refXml)
  {
    Dialog_CopyTranslation* dialog = 0;
    refXml->get_widget_derived("dialog_translation_copy", dialog);
    if(dialog)
    {
      dialog->set_transient_for(*this);
      const int response = Glom::Utils::dialog_run_with_help(dialog, "dialog_translation_copy");
      dialog->hide();

      if(response == Gtk::RESPONSE_OK)
      {
        const Glib::ustring copy_source_locale = dialog->get_locale();
        if(!copy_source_locale.empty())
        {
          //Save and update:
          on_combo_target_locale_changed();

          for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
          {
            Gtk::TreeModel::Row row = *iter;

            sharedptr<TranslatableItem> item = row[m_columns.m_col_item];
            if(item)
            {
              //Copy the translation from the chosen locale to the current locale:
              const Glib::ustring translation = item->get_title(copy_source_locale);
              row[m_columns.m_col_translation] = translation;
            }
          }

          //Save and update:
          m_treeview_modified = true;
          save_to_document();
        }
      }

      delete dialog;
    }
  }
}

void Window_Translations::on_combo_target_locale_changed()
{
  save_to_document();

  m_translation_locale = m_combo_target_locale->get_selected_locale();
  load_from_document();
}

void Window_Translations::on_treeview_edited(const Glib::ustring& /* path */, const Glib::ustring& /* new_text */)
{
  m_treeview_modified = true;
}

static jmp_buf jump;

static void show_gettext_error(int severity, const char* filename, const gchar* message)
{
  std::ostringstream msg_stream;
  if (filename != NULL);
    msg_stream << filename << ": ";
  if (message != NULL)
   msg_stream << message;
  switch (severity)
  {
    #ifdef PO_SEVERITY_WARNING //This was introduced in libgettext-po some time after gettext version 0.14.5 
    case PO_SEVERITY_WARNING:
    {
      // Show only debug output
      std::cout << _("Gettext-Warning: ") << msg_stream.str() << std::endl;
      break;
    }
    #endif //PO_SEVERITY_WARNING


    #ifdef PO_SEVERITY_ERROR //This was introduced in libgettext-po some time after gettext version 0.14.5 
    case PO_SEVERITY_ERROR:
    #endif //PO_SEVERITY_ERROR

    #ifdef PO_SEVERITY_FATAL_ERROR //This was introduced in libgettext-po some time after gettext version 0.14.5 
    case PO_SEVERITY_FATAL_ERROR:
    #endif //PO_SEVERITY_FATAL_ERROR

    default:
    {
      Glib::ustring msg = Glib::ustring(_("Gettext-Error: ")) + " " + msg_stream.str();
      Gtk::MessageDialog dlg(msg, false, Gtk::MESSAGE_ERROR);
      dlg.run();
      break;
    }
  }   
}

/*
 * The exception handling of libgettext-po is very ugly! The following methods are called
 * if an exception occurs and may not return in case of a fatal exception. We use setjmp
 * and longjmp to bypass this and return to the caller
 */
#ifdef HAVE_GETTEXTPO_XERROR
static void on_gettextpo_xerror (int severity, po_message_t message, const char *filename, size_t lineno, size_t column,
		  int multiline_p, const char *message_text)
{
  show_gettext_error(severity, filename, message_text);

  #ifdef PO_SEVERITY_FATAL_ERROR  //This was introduced in libgettext-po some time after gettext version 0.14.5 
  if (severity == PO_SEVERITY_FATAL_ERROR)
    longjmp(jump, 1);
  #endif //PO_SEVERITY_FATAL_ERROR
}

static void on_gettextpo_xerror2 (int severity, po_message_t message1, const char *filename1, size_t lineno1, size_t column1,
		   int multiline_p1, const char *message_text1,
		   po_message_t message2, const char *filename2, size_t lineno2, size_t column2,
		   int multiline_p2, const char *message_text2)
{
  show_gettext_error(severity, filename1, message_text1);
  
  #ifdef PO_SEVERITY_FATAL_ERROR  //This was introduced in libgettext-po some time after gettext version 0.14.5 
  if (severity == PO_SEVERITY_FATAL_ERROR)
    longjmp(jump, 1);
  #endif //PO_SEVERITY_FATAL_ERROR
}
#else //HAVE_GETTEXTPO_XERROR
static void on_gettextpo_error(int status, int errnum, const char * /* format */, ...)
{
  std::cerr << "gettext error (old libgettext-po API): status=" << status << ", errnum=" << errnum << std::endl;
}
#endif //HAVE_GETTEXTPO_XERROR

Glib::ustring Window_Translations::get_po_context_for_item(const sharedptr<TranslatableItem>& item)
{
  // Note that this context string should use English rather than the translated strings,
  // or the context would change depending on the locale of the user doing the export:
  return TranslatableItem::get_translatable_type_name_nontranslated(item->get_translatable_item_type()) + " (" + item->get_name() + ")";
}

void Window_Translations::on_button_export()
{
  if (setjmp(jump) != 0)
    return;  
  
  //Show the file-chooser dialog, to select an output .po file:
  Gtk::FileChooserDialog file_dlg(_("Choose .po File Name"), Gtk::FILE_CHOOSER_ACTION_SAVE);
  
  // Only po files
  Gtk::FileFilter filter;
  filter.set_name(_("Po files"));
  filter.add_pattern("*.po");
  file_dlg.add_filter(filter);

  file_dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  file_dlg.add_button(_("Export"), Gtk::RESPONSE_OK);  
  
  const int result = file_dlg.run();
  if(result == Gtk::RESPONSE_OK)
  {
      std::string filename = file_dlg.get_filename();
      if(filename.empty())
        return;

      //Enforce the file extension:
      const std::string extension = ".po";
      bool add_extension = false;
      if(filename.size() <= extension.size())
         add_extension = true;
      else if(filename.substr(filename.size() - extension.size()) != extension)
         add_extension = true;

      if(add_extension)
        filename += extension;


      po_file_t po_file = po_file_create();
      po_message_iterator_t msg_iter = po_message_iterator(po_file, NULL);
      
      for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
      {
        Gtk::TreeModel::Row row = *iter;

        sharedptr<TranslatableItem> item = row[m_columns.m_col_item];
        if(item)
        {
          po_message_t msg = po_message_create();
          po_message_set_msgid(msg, item->get_title_original().c_str());
          po_message_set_msgstr(msg, item->get_translation(m_translation_locale).c_str());

          // Add "context" comments, to uniquely identify similar strings, used in different places,
          // and to provide a hint for translators.
          po_message_set_msgctxt(msg, get_po_context_for_item(item).c_str());

          po_message_insert(msg_iter, msg);
        }
      }

      po_message_iterator_free(msg_iter);

      #ifdef HAVE_GETTEXTPO_XERROR
      po_xerror_handler error_handler;
      memset(&error_handler, 0, sizeof(error_handler));
      error_handler.xerror = &on_gettextpo_xerror;
      error_handler.xerror2 = &on_gettextpo_xerror2;
      #else
      po_error_handler error_handler;
      memset(&error_handler, 0, sizeof(error_handler));
      error_handler.error = &on_gettextpo_error;
      #endif //HAVE_GETTEXTPO_XERROR

      if(po_file_write(po_file, filename.c_str(), &error_handler))
      {
        po_file_free(po_file);
      }
   }
}

void Window_Translations::on_button_import()
{
  if (setjmp(jump) != 0)
    return;

  Gtk::FileChooserDialog file_dlg(_("Choose .po File Name"), Gtk::FILE_CHOOSER_ACTION_OPEN);
  
  // Only po files
  Gtk::FileFilter filter;
  filter.set_name(_("Po files"));
  filter.add_pattern("*.po");
  file_dlg.add_filter(filter);

  file_dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  //Note to translators: "Import" here is an action verb - it's a button. 
  file_dlg.add_button(_("Import"), Gtk::RESPONSE_OK);
  
  int result = file_dlg.run();
  if (result == Gtk::RESPONSE_OK)
  {
      // We cannot use an uri here:
      const std::string filename = file_dlg.get_filename();
      if(filename.empty())
        return;

      #ifdef HAVE_GETTEXTPO_XERROR
      po_xerror_handler error_handler;
      memset(&error_handler, 0, sizeof(error_handler));
      error_handler.xerror = &on_gettextpo_xerror;
      error_handler.xerror2 = &on_gettextpo_xerror2;
      #else
      po_error_handler error_handler;
      memset(&error_handler, 0, sizeof(error_handler));
      error_handler.error = &on_gettextpo_error;
      #endif //HAVE_GETTEXTPO_XERROR

      po_file_t po_file = po_file_read(filename.c_str(), &error_handler);
      if (!po_file)
      {
        // error message is already given by error_handle.
        return;
      }

      //Look at each domain (could there be more than one?):
      const char* const* domains = po_file_domains(po_file);
      for (int i = 0; domains[i] != NULL; i++)
      {
        //Look at each message:
        po_message_iterator_t iter = po_message_iterator(po_file, domains[i]);
        po_message_t msg;
        while ((msg = po_next_message(iter)))
        {
          //This message:
          //TODO: Just use const char* instead of copying it in to a Glib::ustring,
          //if we have performance problems here:
          const Glib::ustring msgid = po_message_msgid(msg);
          const Glib::ustring msgstr = po_message_msgstr(msg);
          const Glib::ustring msgcontext = po_message_msgctxt(msg);

          //Find the matching entry in the TreeModel:
          for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
          {
            Gtk::TreeModel::Row row = *iter;

            sharedptr<TranslatableItem> item = row[m_columns.m_col_item];
            if(item)
            {
              if( (item->get_title_original() == msgid) && 
                  (get_po_context_for_item(item) == msgcontext) ) // This is not efficient, but it should be reliable.
              {
                item->set_translation(m_translation_locale, msgstr);
                // Keep examining items, in case there are duplicates. break;
              }
            }
          }
        }
        po_message_iterator_free(iter);
      }

      po_file_free(po_file);

      save_to_document();
   }
}

} //namespace Glom
