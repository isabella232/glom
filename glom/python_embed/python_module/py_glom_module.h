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

#ifndef GLOM_PYTHON_GLOM_MODULE_H
#define GLOM_PYTHON_GLOM_MODULE_H

#include <Python.h>

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
//#define PyMODINIT_FUNC void
#endif

/**
 * PyGlomRecord provides
 * - field values: for instance, name = record["name_first"];
 * - relationships (PyGlomRelated): for instance, record.related
 * PyGlomRelated provides:
 * - related records (PyGlomRelatedRecord) - for instance, record.related["contacts"]
 * PyGlomRelatedRecord provides
 * - field values: for instance, name = record.related["contacts"]["name_first"];
 * - Summary functions: for instance, total = record.related["invoice_lines"].sum("price");
 */



#endif //GLOM_PYTHON_GLOM_MODULE_H
