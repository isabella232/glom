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

//We need to include this before anything else, to avoid redefinitions:
#include <boost/python.hpp>
#include <compile.h> /* for the PyCodeObject */
#include <eval.h> /* for PyEval_EvalCode */
#include <objimpl.h> /* for PyObject_New() */

#include <libglom/python_embed/py_glom_record.h>
#include <libglom/python_embed/py_glom_related.h>
#include <libglom/python_embed/py_glom_relatedrecord.h>

BOOST_PYTHON_MODULE(glom)
{
    boost::python::class_<Glom::PyGlomRecord>("Record")
        .add_property("table_name", &Glom::PyGlomRecord::get_table_name)
        .add_property("connection", &Glom::PyGlomRecord::get_connection)
        .add_property("related", &Glom::PyGlomRecord::get_related)

        /* TODO: python still says "TypeError: 'Boost.Python.class' object is unsubscriptable" */
	        /* This suggests that it should work: http://lists.boost.org/boost-users/2003/08/4750.php */
        .def("__getitem__", &Glom::PyGlomRecord::getitem)
        .def("__len__", &Glom::PyGlomRecord::length)
    ;

    boost::python::class_<Glom::PyGlomRelated>("Related")
        .def("__getitem__", &Glom::PyGlomRelated::getitem)
        .def("__len__", &Glom::PyGlomRelated::length)
    ;

    boost::python::class_<Glom::PyGlomRelatedRecord>("RelatedRecord")
        .def("sum", &Glom::PyGlomRelatedRecord::sum, boost::python::args("field_name"), "Add all values of the field in the related records.")
        .def("count", &Glom::PyGlomRelatedRecord::sum, boost::python::args("field_name"), "Count all values in the field in the related records.")
        .def("min", &Glom::PyGlomRelatedRecord::sum, boost::python::args("field_name"), "Minimum of all values of the field in the related recordss.")
        .def("max", &Glom::PyGlomRelatedRecord::sum, boost::python::args("field_name"), "Maximum of all values of the field in the related records.")

        .def("__getitem__", &Glom::PyGlomRecord::getitem)
        .def("__len__", &Glom::PyGlomRecord::length)
    ;
}

