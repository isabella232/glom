/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#include "config.h"
//We need to include this before anything else, to avoid redefinitions:
#include <boost/python.hpp>

#include <libglom/python_embed/py_glom_record.h>
#include <libglom/python_embed/py_glom_related.h>
#include <libglom/python_embed/py_glom_relatedrecord.h>
#include <libglom/python_embed/py_glom_ui.h>

using namespace Glom;

BOOST_PYTHON_MODULE(glom_1_34)
{
  boost::python::docstring_options doc_options(
    true, // show the docstrings from here
    false, // don't show Python signatures.
    false); // Don't mention the C++ method signatures in the generated docstrings.

  // Note: Our python docstring documentation is in ReStructuredText format,
  // because we use sphinx to generate the API reference documentation.
  // However, sphinx does not seem to allow us to use regular reStructuredText
  // - section headings
  // - :: at the end of a paragraph, instead of on a separate line.
  // TODO: Add # targets so we can link directly to parts such as Testing for Empty Values
  // TODO: We can't seem to use reStuctureText sections.

  boost::python::class_<PyGlomRecord>("Record",
    "This is the  current record of the current table. A :class:`Record` object is passed to field calculations and button scripts, providing access to the values of the fields in that record. The :class:`Record` object passed to field calculations is read-only.\n"
    "\n"
    "Field values\n"
    "  Use record['field_name'] to get the value of a specified field in the current record. For instance, this concatenates the values of the name_first and name_last fields in the current record.\n"
    "  ::\n"
    "\n"
    "    record['name_first'] + ' ' + record['name_last']\n"
    "\n"
    "  You may also use this syntax to set the value of a field in the current record. This is possible in a button script, but not in a field calculation. For instance, this sets the value of the name_first field in the current record.\n"
    "  ::\n"
    "\n"
    "    record['name_first'] = 'Bob'\n"
    "\n"
    "Related Records\n"
    "  Use the :attr:`related` attribute to access related records via a relationship."
    "\n"
     "Testing for Empty Values\n"
    "  How you test for empty values depends on the type of field.\n"
    "\n"
    "  Non-Text Fields\n"
    "    Non-text fields may be empty, indicating that the user has not entered any value in the field. For instance, Glom does not assume that an empty value in a numeric field should mean 0. You can test whether a field is empty by using Python's None. For instance.\n"
    "    ::\n"
    "\n"
    "      if(record['contact_id'] == None):\n"
    "          return 'No Contact'\n"
    "      else:\n"
    "          return record.related['contacts']['name_full']\n"
    "\n"
    "  Text Fields\n"
    "    For text fields, you should check for zero-length strings. It is not possible in Glom to distinguish between zero-length strings and the absence of any string, because there is no advantage to doing so. For instance:\n"
    "    ::\n"
    "\n"
    "      if(record['name_full'] == ''):\n"
    "          return 'No Name'\n"
    "      else:\n"
    "          return record['name_full']\n")

    // boost::python doesn't mention the property types in the generated docstring,
    // so we must mention it in the properties' docstring text.
    .add_property("table_name", &PyGlomRecord::get_table_name,
       "The name of the current table as a string.")
    .add_property("connection", &PyGlomRecord::get_connection,
       "The current database connection for use with the gi.repository.Gda API. This is a :class:`Gda.Connection` object.")
    .add_property("related", &PyGlomRecord::get_related,
       ":class:`Related` records. Use the ['relationship_name'] notation with this object.")

    // These can have docstring text too, but sphinx ignores it,
    // so we mention the [] syntax in the class docstring instead.
    // TODO: Ask about that / file a sphinx bug.
    .def("__getitem__", &PyGlomRecord::getitem)
    .def("__setitem__", &PyGlomRecord::setitem)
    .def("__len__", &PyGlomRecord::len)
  ;

  boost::python::class_<PyGlomRelated>("Related",
    "This object provides access to related records via its [] syntax, which takes a relationship name and returns a :class:`RelatedRecord`. For instance, this provides the related records for the current record, via the :attr:`Record.related` attribute\n"
    "::\n"
    "\n"
    "  record.related['location']")

    .def("__getitem__", &PyGlomRelated::getitem)
    .def("__len__", &PyGlomRelated::len)
  ;

  boost::python::class_<PyGlomRelatedRecord>("RelatedRecord",
    "One or more related records, returned by the [] syntax on a :class:`Related` object, such as the :attr:`Record.related` attribute.\n"
    "\n"
    "Single Related Records\n"
    "  For relationships that specify a single record, you can get the value of a field in that record by using the [] synax, providing a field name. For instance, this is the value of the name field in the table indicated by the location relationship (often called location::name).\n"
    "  ::\n"
    "\n"
    "    record.related['location']['name']\n"
    "\n"
    "Multiple Related Records\n"
    "  For relationships that specify multiple records, you can use the aggregate functions to get overall values. For instance, this provides the sum of all total_price values from all of the lines of the current invoice record. See the :class:`RelatedRecord` class for more aggregate functions.\n"
    "  ::\n"
    "\n"
    "    record.related['invoice_lines'].sum('total_price')\n")

    .def("sum", &PyGlomRelatedRecord::sum, boost::python::arg("field_name"),
      "  Add all values of the field in the related records.\n"
      "\n"
      "  :param field_name: The name of the field.\n"
      "  :type field_name: string\n"
      "  :returns: The summarized value.")
    .def("count", &PyGlomRelatedRecord::sum, boost::python::arg("field_name"),
      "  Count all values in the field in the related records.\n"
      "\n"
      "  :param field_name: The name of the field.\n"
      "  :type field_name: string\n"
      "  :returns: The summarized value.")
    .def("min", &PyGlomRelatedRecord::sum, boost::python::arg("field_name"),
      "  Minimum of all values of the field in the related records.\n"
      "\n"
      "  :param field_name: The name of the field.\n"
      "  :type field_name: string\n"
      "  :returns: The summarized value.")
    .def("max", &PyGlomRelatedRecord::sum, boost::python::arg("field_name"),
      "  Maximum of all values of the field in the related records.\n"
      "\n"
      "  :param field_name: The name of the field.\n"
      "  :type field_name: string\n"
      "  :returns: The summarized value.")
    .def("__getitem__", &PyGlomRelatedRecord::getitem)
    .def("__len__", &PyGlomRelatedRecord::len)
  ;

  boost::python::class_<PyGlomUI>("UI",
    "A collection of methods to programatically change the Glom UI, performing some tasks that might otherwise be done by the user via the mouse and keyboard.  A :class:`UI` object is passed to button scripts, allowing them to control the user interface.")

    .def("show_table_details", &PyGlomUI::show_table_details,
      boost::python::arg("table_name"), boost::python::arg("primary_key_value"),
      "  Navigate to the specified table, showing its details view for the specified record.\n"
      "\n"
      "  :param table_name: The name of the table to navigate to.\n"
      "  :type table_name: string\n"
      "  :param primary_key_value: The value of the primary key field in the record to navigate to.")
    .def("show_table_list", &PyGlomUI::show_table_list,
       boost::python::arg("table_name"),
       "  Navigate to the specified table, showing its list view.\n"
      "\n"
       "  :param table_name: The name of the table to navigate to."
       "  :type table_name: string")
    .def("print_layout", &PyGlomUI::print_layout,
      "Print the current layout for the current table.")
    .def("print_report", &PyGlomUI::print_report,
      boost::python::arg("report_name"),
      "  Print the specified report for the current table.\n"
      "\n"
      "  :param report_name: The name of the report to print.\n"
      "  :type report_name: string")
    .def("start_new_record", &PyGlomUI::start_new_record,
      "Start a new empty record for the current table, offering the empty record in the UI.")
  ;
}
