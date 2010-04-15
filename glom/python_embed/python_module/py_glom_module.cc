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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
//We need to include this before anything else, to avoid redefinitions:
#include <boost/python.hpp>

//#include <compile.h> /* for the PyCodeObject */
//#include <eval.h> /* for PyEval_EvalCode */

#include <libglom/python_embed/py_glom_record.h>
#include <libglom/python_embed/py_glom_related.h>
#include <libglom/python_embed/py_glom_relatedrecord.h>
#include <libglom/python_embed/py_glom_ui.h>

using namespace Glom;

BOOST_PYTHON_MODULE(glom_1_14)
{
  boost::python::docstring_options doc_options(
    true, // show the docstrings from here
    true, // show Python signatures.
    false); // Don't mention the C++ method signatures in the generated docstrings.
  boost::python::class_<PyGlomRecord>("Record", 
    "The current record of the current table.\n"
    "Use record['field_name'] to get the value of a specified field in the current record.\n"
    "Use record.related['relationship_name']['field_name'] to get the value of a specified field in a related record.\n"
    "Use record.related['relationship_name'].sum('field_name') to get a sum of all the values of a specified field in multiple related records. See the RelatedRecord object for more aggregate functions."
    )
    .add_property("table_name", &PyGlomRecord::get_table_name,
       "The name of the current table.")
    .add_property("connection", &PyGlomRecord::get_connection,
       "The current database connection for use with the pygda API.")
    .add_property("related", &PyGlomRecord::get_related,
       "Related records. Use the ['relationship_name'] notation with this object.")

    .def("__getitem__", &PyGlomRecord::getitem)
    .def("__setitem__", &PyGlomRecord::setitem)
    .def("__len__", &PyGlomRecord::len)
  ;

  boost::python::class_<PyGlomRelated>("Related")
    .def("__getitem__", &PyGlomRelated::getitem)
    .def("__len__", &PyGlomRelated::len)
  ;

  boost::python::class_<PyGlomRelatedRecord>("RelatedRecord")
    .def("sum", &PyGlomRelatedRecord::sum, boost::python::args("field_name"),
      "Add all values of the field in the related records.")
    .def("count", &PyGlomRelatedRecord::sum, boost::python::args("field_name"),
      "Count all values in the field in the related records.")
    .def("min", &PyGlomRelatedRecord::sum, boost::python::args("field_name"),
      "Minimum of all values of the field in the related recordss.")
    .def("max", &PyGlomRelatedRecord::sum, boost::python::args("field_name"),
      "Maximum of all values of the field in the related records.")
    .def("__getitem__", &PyGlomRelatedRecord::getitem)
    .def("__len__", &PyGlomRelatedRecord::len)
  ;

  boost::python::class_<PyGlomUI>("UI",
    "A collection of methods to programatically change the Glom UI, performing tasks that might otherwise be done by the user via the mouse and keyboard.")
    .def("show_table_details", &PyGlomUI::show_table_details,
      boost::python::args("table_name", "primary_key_value"),
      "Navigate to the specified table, showing its details view for the specified record.")
    .def("show_table_list", &PyGlomUI::show_table_list,
       boost::python::args("table_name"),
      "Navigate to the specified table, showing its list view.")
    .def("print_layout", &PyGlomUI::print_layout,
      "Print the current layout for the current table.")
    .def("print_report", &PyGlomUI::print_report,
      boost::python::args("report_name"),
      "Print the specified report for the current table.")
    .def("start_new_record", &PyGlomUI::start_new_record,
      "Start a new empty record for the current table, offering the empty record in the UI.")
  ;
}
