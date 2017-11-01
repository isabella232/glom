
/* Generated data (by glib-mkenums) */

#ifndef __EPC_ENUMS_H__
#define __EPC_ENUMS_H__

#include <glib-object.h>

G_BEGIN_DECLS
/* Generated by glib-mkenums from <libglom/libepc/dispatcher.h> */
#include <libglom/libepc/dispatcher.h>
#define EPC_TYPE_COLLISION_HANDLING (epc_collision_handling_get_type())

GType                 epc_collision_handling_get_type  (void) G_GNUC_CONST;
GEnumClass*           epc_collision_handling_get_class (void) G_GNUC_CONST;
const gchar* epc_collision_handling_to_string (EpcCollisionHandling value) G_GNUC_PURE;
/* Generated by glib-mkenums from <libglom/libepc/protocol.h> */
#include <libglom/libepc/protocol.h>
#define EPC_TYPE_PROTOCOL (epc_protocol_get_type())

GType                 epc_protocol_get_type  (void) G_GNUC_CONST;
GEnumClass*           epc_protocol_get_class (void) G_GNUC_CONST;
const gchar* epc_protocol_to_string (EpcProtocol value) G_GNUC_PURE;
/* Generated by glib-mkenums from <libglom/libepc/publisher.h> */
#include <libglom/libepc/publisher.h>
#define EPC_TYPE_AUTH_FLAGS (epc_auth_flags_get_type())

GType                 epc_auth_flags_get_type  (void) G_GNUC_CONST;
GFlagsClass*           epc_auth_flags_get_class (void) G_GNUC_CONST;
const gchar* epc_auth_flags_to_string (EpcAuthFlags value) G_GNUC_PURE;
/* Generated by glib-mkenums from <libglom/libepc/service-info.h> */
#include <libglom/libepc/service-info.h>
#define EPC_TYPE_ADDRESS_FAMILY (epc_address_family_get_type())

G_END_DECLS

#endif /* __EPC_ENUMS_H__ */

/* Generated data ends here */

