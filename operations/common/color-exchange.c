/* This file is an image processing operation for GEGL
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
 * Copyright 2014 Christian Busch <christian.busch2@gmx.de>
 *
 * This port is based on the implementation of color-exchange in the gimp source code.
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_CHANT_PROPERTIES

gegl_chant_color  (start_color,  _("Start Color"), "black",
                                 _("The color to be exchanged"))

gegl_chant_int    (red_threshold, _("Red Threshold"), 0, 255, 0,
                                  _("The threshold for the red component of the selecting color."))

gegl_chant_int    (green_threshold, _("Green Threshold"), 0, 255, 0,
                                  _("The threshold for the green component of the selecting color."))

gegl_chant_int    (blue_threshold, _("Blue Threshold"), 0, 255, 0,
                                  _("The threshold for the blue component of the selecting color."))

gegl_chant_color  (end_color,    _("End Color"), "white",
                                 _("The new color."))

#else

#define GEGL_CHANT_TYPE_POINT_FILTER
#define GEGL_CHANT_C_FILE "color-exchange.c"

#include "gegl-chant.h"

static gboolean
process (GeglOperation       *op,
         void                *in_buf,
         void                *out_buf,
         glong                samples,
         const GeglRectangle *roi,
         gint                 level)
{ 
  GeglChantO *o = GEGL_CHANT_PROPERTIES (op);

  guint8     * GEGL_ALIGNED in_pixel;
  guint8     * GEGL_ALIGNED out_pixel;

  guint8      colorFrom[4];
  guint8      colorTo[4];
  guint8      threshold[4];
  guint8      delta[4];

  in_pixel   = in_buf;
  out_pixel  = out_buf;

  gegl_color_get_pixel (o->start_color, babl_format ("RGBA u8"), colorFrom);
  gegl_color_get_pixel (o->end_color, babl_format ("RGBA u8"), colorTo);

  threshold[0] = o->red_threshold;
  threshold[1] = o->blue_threshold;
  threshold[2] = o->green_threshold;

  for (glong i = 0; i < samples; i++)
    {
      if (in_pixel[0] >= CLAMP (colorFrom[0] - threshold[0], 0, 255) &&
          in_pixel[0] <= CLAMP (colorFrom[0] + threshold[0], 0, 255) &&
          in_pixel[1] >= CLAMP (colorFrom[1] - threshold[1], 0, 255) &&
          in_pixel[1] <= CLAMP (colorFrom[1] + threshold[1], 0, 255) &&
          in_pixel[2] >= CLAMP (colorFrom[2] - threshold[2], 0, 255) &&
          in_pixel[2] <= CLAMP (colorFrom[2] + threshold[2], 0, 255))
        {
          delta[0] = in_pixel[0] > colorFrom[0] ?
            in_pixel[0] - colorFrom[0] : colorFrom[0] - in_pixel[0];
          delta[1] = in_pixel[1] > colorFrom[1] ?
            in_pixel[1] - colorFrom[1] : colorFrom[1] - in_pixel[1];
          delta[2] = in_pixel[2] > colorFrom[2] ?
            in_pixel[2] - colorFrom[2] : colorFrom[2] - in_pixel[2];

          out_pixel[0] = CLAMP (colorTo[0] + delta[0], 0, 255);
          out_pixel[1] = CLAMP (colorTo[1] + delta[1], 0, 255);
          out_pixel[2] = CLAMP (colorTo[2] + delta[2], 0, 255);
        }
      else
        {
          out_pixel[0] = in_pixel[0];
          out_pixel[1] = in_pixel[1];
          out_pixel[2] = in_pixel[2];
        }

      out_pixel[3] = in_pixel[3];

      in_pixel += 4;
      out_pixel += 4;
    }

  return TRUE;

}

static void
prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "input",  babl_format ("RGBA u8"));
  gegl_operation_set_format (operation, "output", babl_format ("RGBA u8"));
}

static void
gegl_chant_class_init (GeglChantClass *klass)
{
  GeglOperationClass            *operation_class;
  GeglOperationPointFilterClass *point_filter_class;

  operation_class    = GEGL_OPERATION_CLASS (klass);
  point_filter_class = GEGL_OPERATION_POINT_FILTER_CLASS (klass);

  operation_class->prepare = prepare;
  point_filter_class->process = process;

  gegl_operation_class_set_keys (operation_class,
    "name"        , "gegl:color-exchange",
    "categories"  , "colors/map",
    "description" , _("Exchanges one color with another."),
    NULL);
}

#endif
