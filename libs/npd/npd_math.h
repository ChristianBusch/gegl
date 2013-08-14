/*
 * This file is part of n-point image deformation library.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2013 Marek Dvoroznak <dvoromar@gmail.com>
 */

#ifndef __NPD_MATH_H__
#define	__NPD_MATH_H__

#include "npd_common.h"

#define NPD_EPSILON 0.00001

void        npd_compute_homography   (NPDPoint  *p11,
                                      NPDPoint  *p21,
                                      NPDPoint  *p31,
                                      NPDPoint  *p41,
                                      NPDPoint  *p12,
                                      NPDPoint  *p22,
                                      NPDPoint  *p32,
                                      NPDPoint  *p42,
                                      NPDMatrix *T);
void        npd_compute_affinity     (NPDPoint  *p11,
                                      NPDPoint  *p21,
                                      NPDPoint  *p31,
                                      NPDPoint  *p12,
                                      NPDPoint  *p22,
                                      NPDPoint  *p32,
                                      NPDMatrix *T);
void        npd_apply_transformation (NPDMatrix *T,
                                      NPDPoint  *src,
                                      NPDPoint  *dest);
gboolean    npd_equal_floats_epsilon (gfloat a,
                                      gfloat b,
                                      gfloat epsilon);
gboolean    npd_equal_floats         (gfloat a,
                                      gfloat b);
gfloat      npd_SED                  (NPDPoint *p1,
                                      NPDPoint *p2);
void        npd_new_matrix           (NPDMatrix **matrix);
void        npd_destroy_matrix       (NPDMatrix **matrix);

#endif	/* __NPD_MATH_H__ */