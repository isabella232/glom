/***************************************************************************
                          notebook_design.h  -  description
                             -------------------
    begin                : Fri Aug 11 2000
    copyright            : (C) 2000 by Murray Cumming
    email                : murrayc@usa.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NOTEBOOK_DESIGN_H
#define NOTEBOOK_DESIGN_H

#include "../notebook_glom.h"
#include "fields/box_db_table_definition.h"
#include "box_db_table_relationships.h"

/**
  *@author Murray Cumming
  */

class Notebook_Design : public Notebook_Glom
{
public: 
  Notebook_Design();
  virtual ~Notebook_Design();
    
  virtual void initialize(const Glib::ustring& strDatabaseName, const Glib::ustring& strTableName);
  
protected:

  //Member widgets:
  Box_DB_Table_Definition m_Box_Fields;
  Box_DB_Table_Relationships m_Box_Relationships;
};

#endif
