/* This file is the public GEGL API
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
 * 2000-2008 © Calvin Williamson, Øyvind Kolås.
 */

#ifndef __GEGL_OPERATIONS_UTIL_H__
#define __GEGL_OPERATIONS_UTIL_H__

#if !defined (__GEGL_H_INSIDE__)
#error "This file can not be included directly, use <gegl.h> instead."
#endif

G_BEGIN_DECLS

/***
 * Available operations:
 * Gegl provides means to check for available processing operations that
 * can be used with nodes using #gegl_list_operations and #gegl_has_operation.
 * For a specified op, you can get a list of properties with
 * #gegl_operation_list_properties.
 */

/**
 * gegl_list_operations:
 * @n_operations_p: (out caller-allocates): return location for number of operations.
 *
 * Return value: (transfer container) (array length=n_operations_p): An
 * alphabetically sorted array of available operation names. This excludes any
 * compat-name registered by operations. The list should be freed with g_free
 * after use.
 * ---
 * gchar **operations;
 * guint   n_operations;
 * gint i;
 *
 * operations = gegl_list_operations (&n_operations);
 * g_print ("Available operations:\n");
 * for (i=0; i < n_operations; i++)
 *   {
 *     g_print ("\t%s\n", operations[i]);
 *   }
 * g_free (operations);
 */
gchar        **gegl_list_operations         (guint *n_operations_p);

/**
 * gegl_has_operation:
 * @operation_type: the name of the operation
 *
 * Return value: A boolean telling whether the operation is present or not. This
 * also returns true for any compat-name registered by operations.
 */

gboolean       gegl_has_operation           (const gchar *operation_type);

/**
 * gegl_operation_list_properties:
 * @operation_type: the name of the operation type we want to query to properties of.
 * @n_properties_p: (out caller-allocates): return location for number of properties.
 *
 * Return value: (transfer container) (array length=n_properties_p): An allocated array of #GParamSpecs describing the properties
 * of the operation available when a node has operation_type set. The list should be freed with g_free after use.
 */
GParamSpec** gegl_operation_list_properties (const gchar   *operation_type,
                                             guint         *n_properties_p);

G_END_DECLS

#endif /* __GEGL_OPERATIONS_UTIL_H__ */