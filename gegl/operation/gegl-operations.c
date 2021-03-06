/* This file is part of GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2003 Calvin Williamson
 *           2005-2008 Øyvind Kolås
 *           2013 Michael Henning
 */

#include "config.h"

#include <glib-object.h>
#include <string.h>

#include "gegl.h"
#include "gegl-types-internal.h"
#include "gegl-debug.h"
#include "gegl-operation.h"
#include "gegl-operations.h"
#include "gegl-operation-context.h"


static GHashTable *gtype_hash        = NULL;
static GSList     *operations_list   = NULL;
static guint       gtype_hash_serial = 0;
G_LOCK_DEFINE_STATIC (gtype_hash);

void
gegl_operation_class_register_name (GeglOperationClass *klass,
                                    const gchar        *name,
                                    const gboolean      is_compat)
{
  GType this_type, check_type;
  this_type = G_TYPE_FROM_CLASS (klass);

  G_LOCK (gtype_hash);

  check_type = (GType) g_hash_table_lookup (gtype_hash, name);
  if (check_type && check_type != this_type)
    {
      g_warning ("Adding %s shadows %s for operation %s",
                  g_type_name (this_type),
                  g_type_name (check_type),
                  name);
    }
  g_hash_table_insert (gtype_hash, g_strdup (name), (gpointer) this_type);

  if (!check_type && !is_compat)
    operations_list = g_slist_insert_sorted (operations_list, (gpointer) name,
                                             (GCompareFunc) strcmp);

  G_UNLOCK (gtype_hash);
}

static void
add_operations (GType parent)
{
  GType *types;
  guint  count;
  guint  no;

  types = g_type_children (parent, &count);
  if (!types)
    return;

  for (no = 0; no < count; no++)
    {
      /*
       * Poke the operation so it registers its name with
       * gegl_operation_class_register_name
       */
      g_type_class_unref (g_type_class_ref (types[no]));

      add_operations (types[no]);
    }
  g_free (types);
}

GType
gegl_operation_gtype_from_name (const gchar *name)
{
  /* If any new modules have been loaded, scan for GeglOperations */
  guint latest_serial;
  latest_serial = g_type_get_type_registration_serial ();
  if (gtype_hash_serial != latest_serial)
    {
      add_operations (GEGL_TYPE_OPERATION);
      gtype_hash_serial = latest_serial;
    }

  return (GType) g_hash_table_lookup (gtype_hash, name);
}

gboolean
gegl_has_operation (const gchar *operation_type)
{
  return gegl_operation_gtype_from_name (operation_type) != 0;
}

gchar **gegl_list_operations (guint *n_operations_p)
{
  gchar **pasp = NULL;
  gint    n_operations;
  gint    i;
  GSList *iter;
  gint    pasp_size = 0;
  gint    pasp_pos;

  if (!operations_list)
    {
      gegl_operation_gtype_from_name ("");

      /* should only happen if no operations are found */
      if (!operations_list)
        {
          if (n_operations_p)
            *n_operations_p = 0;
          return NULL;
        }
    }

  n_operations = g_slist_length (operations_list);
  pasp_size   += (n_operations + 1) * sizeof (gchar *);
  for (iter = operations_list; iter != NULL; iter = g_slist_next (iter))
    {
      const gchar *name = iter->data;
      pasp_size += strlen (name) + 1;
    }
  pasp     = g_malloc (pasp_size);
  pasp_pos = (n_operations + 1) * sizeof (gchar *);
  for (iter = operations_list, i = 0; iter != NULL; iter = g_slist_next (iter), i++)
    {
      const gchar *name = iter->data;
      pasp[i] = ((gchar *) pasp) + pasp_pos;
      strcpy (pasp[i], name);
      pasp_pos += strlen (name) + 1;
    }
  pasp[i] = NULL;
  if (n_operations_p)
    *n_operations_p = n_operations;
  return pasp;
}

void
gegl_operation_gtype_init (void)
{
  G_LOCK (gtype_hash);

  if (!gtype_hash)
    gtype_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  G_UNLOCK (gtype_hash);
}

void
gegl_operation_gtype_cleanup (void)
{
  G_LOCK (gtype_hash);
  if (gtype_hash)
    {
      g_hash_table_destroy (gtype_hash);
      gtype_hash = NULL;
    }
  G_UNLOCK (gtype_hash);
}

gboolean gegl_can_do_inplace_processing (GeglOperation       *operation,
                                         GeglBuffer          *input,
                                         const GeglRectangle *result)
{
  if (!input)
    return FALSE;
  if (gegl_object_get_has_forked (G_OBJECT (input)))
    return FALSE;

  if (gegl_buffer_get_format (input) == gegl_operation_get_format (operation, "output") &&
      gegl_rectangle_contains (gegl_buffer_get_extent (input), result))
    return TRUE;
  return FALSE;
}

static GQuark
gegl_has_forked_quark (void)
{
  static GQuark the_quark = 0;

  if (G_UNLIKELY (the_quark == 0))
    the_quark = g_quark_from_static_string ("gegl-object-has-forked");

  return the_quark;
}

void
gegl_object_set_has_forked (GObject *object)
{
  g_object_set_qdata (object, gegl_has_forked_quark (), (void*)0xf);
}


gboolean
gegl_object_get_has_forked (GObject *object)
{
  return g_object_get_qdata (object, gegl_has_forked_quark ()) != NULL;
}
