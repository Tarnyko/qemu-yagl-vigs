/*
 * yagl
 *
 * Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact:
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

#ifndef _QEMU_YAGL_GLES_API_TS_H
#define _QEMU_YAGL_GLES_API_TS_H

#include "yagl_types.h"

struct yagl_gles_driver;
struct yagl_gles_api_ps;
struct yagl_gles_api;

/*
 * OpenGL 3.1+ core profile doesn't allow one to
 * call glVertexAttribPointer and friends without a VBO, thus,
 * we need to have a backing VBO in order to support GLESv2.
 */
struct yagl_gles_array
{
    GLuint vbo;
    uint32_t size;
};

struct yagl_gles_api_ts
{
    struct yagl_gles_driver *driver;

    struct yagl_gles_api_ps *ps;

    /*
     * From 'ps->base.api' for speed.
     */
    struct yagl_gles_api *api;

    struct yagl_gles_array *arrays;
    uint32_t num_arrays;

    /*
     * See above comments regarding VBO.
     */
    GLuint ebo;
    uint32_t ebo_size;
};

void yagl_gles_api_ts_init(struct yagl_gles_api_ts *gles_api_ts,
                           struct yagl_gles_driver *driver,
                           struct yagl_gles_api_ps *ps);

void yagl_gles_api_ts_cleanup(struct yagl_gles_api_ts *gles_api_ts);

#endif
