#ifndef _EGG_SPREADTABLE_P_H
#define _EGG_SPREADTABLE_P_H


#include <gtkmm/private/container_p.h>

#include <glibmm/class.h>

namespace Egg
{

class SpreadTable_Class : public Glib::Class
{
public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef SpreadTable CppObjectType;
  typedef EggSpreadTable BaseObjectType;
  typedef EggSpreadTableClass BaseClassType;
  typedef Gtk::Container_Class CppClassParent;
  typedef GtkContainerClass BaseClassParent;

  friend class SpreadTable;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  const Glib::Class& init();


  static void class_init_function(void* g_class, void* class_data);

  static Glib::ObjectBase* wrap_new(GObject*);

protected:

  //Callbacks (default signal handlers):
  //These will call the *_impl member methods, which will then call the existing default signal callbacks, if any.
  //You could prevent the original default signal handlers being called by overriding the *_impl method.

  //Callbacks (virtual functions):
};


} // namespace Egg


#endif /* _EGG_SPREADTABLE_P_H */
