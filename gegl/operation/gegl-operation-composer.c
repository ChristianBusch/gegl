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
 * Copyright 2006 Øyvind Kolås
 */

#include "config.h"

#include <glib-object.h>
#include <string.h>

#include "gegl.h"
#include "gegl-operation-composer.h"
#include "gegl-operation-context.h"

static gboolean gegl_operation_composer_process (GeglOperation       *operation,
                              GeglOperationContext     *context,
                              const gchar         *output_prop,
                              const GeglRectangle *result,
                              gint                 level);
static void     attach       (GeglOperation       *operation);
static GeglNode*detect       (GeglOperation       *operation,
                              gint                 x,
                              gint                 y);

static GeglRectangle get_bounding_box        (GeglOperation        *self);
static GeglRectangle get_required_for_output (GeglOperation        *self,
                                               const gchar         *input_pad,
                                               const GeglRectangle *roi);

G_DEFINE_TYPE (GeglOperationComposer, gegl_operation_composer,
               GEGL_TYPE_OPERATION)


static void
gegl_operation_composer_class_init (GeglOperationComposerClass * klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->process = gegl_operation_composer_process;
  operation_class->attach = attach;
  operation_class->detect = detect;
  operation_class->get_bounding_box = get_bounding_box;
  operation_class->get_required_for_output = get_required_for_output;
}

static void
gegl_operation_composer_init (GeglOperationComposer *self)
{
}

static void
attach (GeglOperation *self)
{
  GeglOperation *operation = GEGL_OPERATION (self);
  GParamSpec    *pspec;

  pspec = g_param_spec_object ("output",
                               "Output",
                               "Output pad for generated image buffer.",
                               GEGL_TYPE_BUFFER,
                               G_PARAM_READABLE |
                               GEGL_PARAM_PAD_OUTPUT);
  gegl_operation_create_pad (operation, pspec);
  g_param_spec_sink (pspec);

  pspec = g_param_spec_object ("input",
                               "Input",
                               "Input pad, for image buffer input.",
                               GEGL_TYPE_BUFFER,
                               G_PARAM_READWRITE |
                               GEGL_PARAM_PAD_INPUT);
  gegl_operation_create_pad (operation, pspec);
  g_param_spec_sink (pspec);

  pspec = g_param_spec_object ("aux",
                               "Aux",
                               "Auxiliary image buffer input pad.",
                               GEGL_TYPE_BUFFER,
                               G_PARAM_READWRITE |
                               GEGL_PARAM_PAD_INPUT);
  gegl_operation_create_pad (operation, pspec);
  g_param_spec_sink (pspec);
}

static gboolean
gegl_operation_composer_process (GeglOperation        *operation,
                                 GeglOperationContext *context,
                                 const gchar          *output_prop,
                                 const GeglRectangle  *result,
                                 gint                  level)
{
  GeglOperationComposerClass *klass   = GEGL_OPERATION_COMPOSER_GET_CLASS (operation);
  GeglBuffer                 *input;
  GeglBuffer                 *aux;
  GeglBuffer                 *output;
  gboolean                    success = FALSE;

  if (strcmp (output_prop, "output"))
    {
      g_warning ("requested processing of %s pad on a composer", output_prop);
      return FALSE;
    }

  input = gegl_operation_context_get_source (context, "input");
  aux   = gegl_operation_context_get_source (context, "aux");
  output = gegl_operation_context_get_target (context, "output");

  /* A composer with a NULL aux, can still be valid, the
   * subclass has to handle it.
   */
  if (input != NULL ||
      aux != NULL)
    {
      success = klass->process (operation, input, aux, output, result, level);

      if (input)
        g_object_unref (input);
      if (aux)
        g_object_unref (aux);
    }
  else
    {
      g_warning ("%s received NULL input and aux",
                 gegl_node_get_operation (operation->node));
    }

  return success;
}

static GeglRectangle
get_bounding_box (GeglOperation *self)
{
  GeglRectangle  result   = { 0, 0, 0, 0 };
  GeglRectangle *in_rect  = gegl_operation_source_get_bounding_box (self, "input");
  GeglRectangle *aux_rect = gegl_operation_source_get_bounding_box (self, "aux");

  if (!in_rect)
    {
      if (aux_rect)
        return *aux_rect;
      return result;
    }
  if (aux_rect)
    {
      gegl_rectangle_bounding_box (&result, in_rect, aux_rect);
    }
  else
    {
      return *in_rect;
    }
  return result;
}

static GeglRectangle
get_required_for_output (GeglOperation        *self,
                         const gchar         *input_pad,
                         const GeglRectangle *roi)
{
  return *roi;
}

static GeglNode *
detect (GeglOperation *operation,
        gint           x,
        gint           y)
{
  GeglNode *input_node = gegl_operation_get_source_node (operation, "input");
  GeglNode *aux_node   = gegl_operation_get_source_node (operation, "aux");

  if (input_node)
    input_node = gegl_node_detect (input_node, x, y);
  if (aux_node)
    aux_node = gegl_node_detect (aux_node, x, y);

  if (aux_node)
    return aux_node;
  if (input_node)
    return input_node;
  return NULL;
}
