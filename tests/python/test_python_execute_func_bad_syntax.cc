#include <glom/libglom/init.h>
#include <glom/python_embed/glom_python.h>
#include <libglom/data_structure/glomconversions.h>
#include <boost/python.hpp>
#include <iostream>

int main()
{
  Glom::libglom_init(); //Also initializes python.

  const char* calculation =
    "count = 0\n"
    "return cownt";
  Glom::type_map_fields field_values;
  Glib::RefPtr<Gnome::Gda::Connection> connection;

  //Execute a python function:
  Gnome::Gda::Value value;
  Glib::ustring error_message;
  try
  {
    value = Glom::glom_evaluate_python_function_implementation(
      Glom::Field::glom_field_type::NUMERIC, calculation, field_values,
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
  //std::cout << "value=" << value.to_string() << std::endl;

  //Check that we received a traceback error:
  g_assert(!error_message.empty());

  //Check that the return value is of the expected type:
  g_assert(
    (value.get_value_type() == GDA_TYPE_NUMERIC) ||
    value.is_null() );

  //Check that the return value is of the expected value:
  const auto numeric = Glom::Conversions::get_double_for_gda_value_numeric(value);
  //std::cout << "numeric=" << value.to_string() << std::endl;
  g_assert(numeric == 0);

  return EXIT_SUCCESS;
}
