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

#ifndef _QEMU_YAGL_VECTOR_H
#define _QEMU_YAGL_VECTOR_H

#include "yagl_types.h"

struct yagl_vector
{
    void *data;
    int elem_size;
    int size;
    int capacity;
};

void yagl_vector_init(struct yagl_vector *v,
                      int elem_size,
                      int initial_capacity);

/*
 * Cleans up the vector, you must call 'yagl_vector_init' again to be able
 * to use it.
 */
void yagl_vector_cleanup(struct yagl_vector *v);

/*
 * Detaches the buffer from vector. vector will automatically clean up,
 * you must call 'yagl_vector_init' again to be able to use it.
 */
void *yagl_vector_detach(struct yagl_vector *v);

int yagl_vector_size(struct yagl_vector *v);

int yagl_vector_capacity(struct yagl_vector *v);

void yagl_vector_push_back(struct yagl_vector *v, const void *elem);

void yagl_vector_resize(struct yagl_vector *v, int new_size);

void *yagl_vector_data(struct yagl_vector *v);

#endif
