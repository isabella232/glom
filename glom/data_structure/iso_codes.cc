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

#include "iso_codes.h"
#include <libxml++/libxml++.h>
#include "../document/document_glom.h"
#include <glibmm/i18n.h>
#include "config.h" //For ISO_CODES_PREFIX.

namespace IsoCodes
{

static type_list_currencies list_currencies;

type_list_currencies get_list_of_currency_symbols()
{
  if(list_currencies.empty())
  {
    const Glib::ustring filename = ISO_CODES_PREFIX "/share/xml/iso-codes/iso_4217.xml";

    try
    {
      xmlpp::DomParser parser;
      //parser.set_validate();
      parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
      parser.parse_file(filename);
      if(parser)
      {
        //Walk the tree:
        const xmlpp::Node* nodeRoot = parser.get_document()->get_root_node(); //deleted by DomParser.

        xmlpp::Node::NodeList listNodes = nodeRoot->get_children("iso_4217_entry");
        for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
        {
          xmlpp::Element* nodeEntry = dynamic_cast<xmlpp::Element*>(*iter);
          if(nodeEntry)
          {
            Currency currency;

            currency.m_symbol = nodeEntry->get_attribute("letter_code")->get_value();

            Glib::ustring name = _(nodeEntry->get_attribute("currency_name")->get_value().c_str());
            const char* pchTranslatedName = dgettext("iso_4217", name.c_str());
            if(pchTranslatedName)
              name = pchTranslatedName;

            currency.m_name = name;

            list_currencies.push_back(currency);
          }
        }
      }
    }
    catch(const std::exception& ex)
    {
      std::cerr << "Exception while parsing iso codes: " << ex.what() << std::endl;
    }
  }

  return list_currencies;
}

} //namespace IsoCodes

