#include <glom/utility_widgets/db_adddel/liststore_with_addrow.h>
#include <gtkmm.h>

int main(int argc, char* argv[])
{
  Gtk::Main kit(argc, argv);

  Gtk::TreeModelColumnRecord columns;
  Glom::ListStoreWithAddRow::type_vec_fields column_fields;
  Glib::RefPtr<Glom::ListStoreWithAddRow> model = Glom::ListStoreWithAddRow::create(columns, column_fields);

  Glib::RefPtr<Gtk::TreeModel> treemodel = Glib::RefPtr<Gtk::TreeModel>::cast_dynamic(model);
  std::cout << "treemodel=" << treemodel << std::endl;
}
