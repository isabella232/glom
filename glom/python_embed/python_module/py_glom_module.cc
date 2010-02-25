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

using namespace Glom;

BOOST_PYTHON_MODULE(glom_1_14)
{
  boost::python::class_<PyGlomRecord>("Record")
    .add_property("table_name", &PyGlomRecord::get_table_name)
    .add_property("connection", &PyGlomRecord::get_connection)
    .add_property("related", &PyGlomRecord::get_related)

    .def("__getitem__", &PyGlomRecord::getitem)
    .def("__setitem__", &PyGlomRecord::setitem)
    .def("__len__", &PyGlomRecord::len)
  ;

  boost::python::class_<PyGlomRelated>("Related")
    .def("__getitem__", &PyGlomRelated::getitem)
    .def("__len__", &PyGlomRelated::len)
  ;

  boost::python::class_<PyGlomRelatedRecord>("RelatedRecord")
    .def("sum", &PyGlomRelatedRecord::sum, boost::python::args("field_name"), "Add all values of the field in the related records.")
    .def("count", &PyGlomRelatedRecord::sum, boost::python::args("field_name"), "Count all values in the field in the related records.")
    .def("min", &PyGlomRelatedRecord::sum, boost::python::args("field_name"), "Minimum of all values of the field in the related recordss.")
    .def("max", &PyGlomRelatedRecord::sum, boost::python::args("field_name"), "Maximum of all values of the field in the related records.")
    .def("__getitem__", &PyGlomRelatedRecord::getitem)
    .def("__len__", &PyGlomRelatedRecord::len)
  ;
}
