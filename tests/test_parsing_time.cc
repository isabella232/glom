#include <libglom/data_structure/glomconversions.h>
#include <libgdamm/init.h>
#include <iostream>

int main()
{
  Gnome::Gda::init();

  const Glib::ustring time_text_input = "01:00 PM";
  //std::cout << "time_text_input=" << time_text_input << std::endl;

  bool success = false;

  //We try parse_time() though parse_value() calls it anyway,
  //to give us a clue if parse_value would fail.
  /* struct tm value_as_tm = */
    Glom::Conversions::parse_time(time_text_input, success);
  if(!success)
  {
    std::cerr << G_STRFUNC << ": Failed: parse_time() failed." << std::endl;
    return EXIT_FAILURE;
  }

  success = false;
  const Gnome::Gda::Value value =
    Glom::Conversions::parse_value(Glom::Field::TYPE_TIME, time_text_input, success);

  if(!success)
  {
    std::cerr << G_STRFUNC << ": Failed: parse_value() failed." << std::endl;
    return EXIT_FAILURE;
  }

  const Gnome::Gda::Time parsed_time = value.get_time();
  //std::cout << "debug: Parsed Time: hour=" <<  parsed_time.hour << ", minute=" <<  parsed_time.minute << ", second=" <<  parsed_time.second << std::endl;

  if(parsed_time.hour != 13)
  {
    std::cerr << G_STRFUNC << ": Failed: The parsed hour was " <<  parsed_time.hour << " instead of 13" << std::endl;
    return EXIT_FAILURE; //Failed.
  }

  if(parsed_time.minute != 0)
  {
    std::cerr << G_STRFUNC << ": Failed: The parsed minute was " <<  parsed_time.minute << " instead of 0" << std::endl;
    return EXIT_FAILURE;
  }

  const Glib::ustring time_text_parsed =
    Glom::Conversions::get_text_for_gda_value(Glom::Field::TYPE_TIME, value);


  //std::cout << "time_text_parsed=" << time_text_parsed << std::endl;
  return EXIT_SUCCESS;

  //This extra check would fail if :00 seconds are added to the text:
  //if(time_text_input == time_text_parsed)
  //   return EXIT_SUCCESS;
  //else
  //   return EXIT_FAILURE;
}
