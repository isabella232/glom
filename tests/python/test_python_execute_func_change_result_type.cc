#include <glom/libglom/init.h>
#include <glom/python_embed/glom_python.h>
#include <libglom/data_structure/glomconversions.h>
#include <boost/python.hpp>
#include <iostream>

int main()
{
  Glom::libglom_init(); //Also initializes python.

  const char* calculation = "count = 0\nfor i in range(0, 100): count += i\nreturn count";
  Glom::type_map_fields field_values;
  Glib::RefPtr<Gnome::Gda::Connection> connection;

  //Execute a python function:
  Gnome::Gda::Value value;
  Glib::ustring error_message;
  const Glom::Field::glom_field_type result_type = Glom::Field::TYPE_TEXT;
  try
  {
    //We ask for a text result though the python function actually returns a number.
    value = Glom::glom_evaluate_python_function_implementation(
      result_type, calculation, field_values,
      0 /* document */, "" /* table name */,
      std::shared_ptr<Glom::Field>(), Gnome::Gda::Value(), // primary key details. Not used in this test.
      connection,
      error_message);
  }
  catch(const std::exception& ex)
  {
    std::cerr << G_STRFUNC << ": Exception: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(const boost::python::error_already_set& ex)
  {
    std::cerr << G_STRFUNC << ": Exception: boost::python::error_already_set" << std::endl;
    return EXIT_FAILURE;
  }

  //std::cout << "type=" << g_type_name(value.get_value_type()) << std::endl;

  //Check that there was no python error:
  g_assert(error_message.empty());

  //Check that the return value is of the expected type:
  g_assert(Glom::Field::get_glom_type_for_gda_type(value.get_value_type()) == result_type);

  //Check that the return value is of the expected value:
  const Glib::ustring text = value.get_string();
  //std::cout << "text=" << text << std::endl;
  g_assert(text == "4950"); //This should always be as per ISO, not according to the user's locale, because it's generally passed to the database. Presentation is separate to calculation or storage.

  //std::cout << "value=" << value.to_string() << std::endl;

  return EXIT_SUCCESS;
}
