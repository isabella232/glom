INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES)

SUBDIRS = converter detail object

noinst_LIBRARIES = libpython_embed_boost_python.a
libpython_embed_boost_python_a_SOURCES = arg_from_python.hpp exception_translator.hpp object_fwd.hpp \
  return_by_value.hpp args_fwd.hpp extract.hpp object.hpp return_internal_reference.hpp \
  args.hpp handle_fwd.hpp object_items.hpp return_opaque_pointer.hpp \
  back_reference.hpp handle.hpp object_operators.hpp return_value_policy.hpp \
  bases.hpp has_back_reference.hpp object_protocol_core.hpp scope.hpp \
  base_type_traits.hpp implicit.hpp object_protocol.hpp self.hpp \
  borrowed.hpp init.hpp object_slices.hpp signature.hpp \
  call.hpp instance_holder.hpp opaque_pointer_converter.hpp slice.hpp \
  call_method.hpp iterator.hpp operators.hpp slice_nil.hpp \
  cast.hpp list.hpp other.hpp str.hpp \
  class_fwd.hpp long.hpp overloads.hpp tag.hpp \
  class.hpp lvalue_from_pytype.hpp override.hpp to_python_converter.hpp \
  copy_const_reference.hpp make_constructor.hpp pointee.hpp to_python_indirect.hpp \
  copy_non_const_reference.hpp make_function.hpp proxy.hpp to_python_value.hpp \
  data_members.hpp manage_new_object.hpp ptr.hpp tuple.hpp \
  default_call_policies.hpp module.hpp pure_virtual.hpp type_id.hpp \
  def.hpp module_init.hpp raw_function.hpp with_custodian_and_ward.hpp \
  def_visitor.hpp numeric.hpp refcount.hpp wrapper.hpp \
  dict.hpp object_attributes.hpp reference_existing_object.hpp \
  enum.hpp object_call.hpp register_ptr_to_python.hpp \
  errors.hpp object_core.hpp return_arg.hpp

libpython_embed_boost_python_a_LIBADD = detail/libpython_embed_boost_python_detail.a
