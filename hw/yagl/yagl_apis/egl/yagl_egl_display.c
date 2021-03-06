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

#include "yagl_egl_display.h"
#include "yagl_egl_backend.h"
#include "yagl_eglb_display.h"
#include "yagl_egl_config.h"
#include "yagl_egl_surface.h"
#include "yagl_egl_context.h"
#include "yagl_process.h"
#include "yagl_thread.h"
#include "yagl_log.h"
#include "yagl_handle_gen.h"

struct yagl_egl_display
    *yagl_egl_display_create(struct yagl_egl_backend *backend,
                             uint32_t display_id)
{
    struct yagl_eglb_display *backend_dpy;
    struct yagl_egl_display *dpy;

    backend_dpy = backend->create_display(backend);

    if (!backend_dpy) {
        return NULL;
    }

    dpy = g_malloc0(sizeof(*dpy));

    dpy->backend = backend;
    dpy->display_id = display_id;
    dpy->handle = yagl_handle_gen();
    dpy->backend_dpy = backend_dpy;

    dpy->initialized = false;

    yagl_resource_list_init(&dpy->configs);
    yagl_resource_list_init(&dpy->contexts);
    yagl_resource_list_init(&dpy->surfaces);

    return dpy;
}

void yagl_egl_display_destroy(struct yagl_egl_display *dpy)
{
    yagl_egl_display_terminate(dpy);

    yagl_resource_list_cleanup(&dpy->surfaces);
    yagl_resource_list_cleanup(&dpy->contexts);
    yagl_resource_list_cleanup(&dpy->configs);

    dpy->backend_dpy->destroy(dpy->backend_dpy);

    g_free(dpy);
}

void yagl_egl_display_initialize(struct yagl_egl_display *dpy)
{
    struct yagl_egl_config **cfgs;
    int i, num_configs = 0;

    YAGL_LOG_FUNC_ENTER(yagl_egl_display_initialize, NULL);

    if (dpy->initialized) {
        goto out;
    }

    cfgs = yagl_egl_config_enum(dpy, &num_configs);

    if (num_configs <= 0) {
        goto out;
    }

    yagl_egl_config_sort(cfgs, num_configs);

    for (i = 0; i < num_configs; ++i) {
        yagl_resource_list_add(&dpy->configs, &cfgs[i]->res);
        yagl_egl_config_release(cfgs[i]);
    }

    g_free(cfgs);

    YAGL_LOG_TRACE("initialized with %d configs", num_configs);

out:
    dpy->initialized = true;

    YAGL_LOG_FUNC_EXIT(NULL);
}

bool yagl_egl_display_is_initialized(struct yagl_egl_display *dpy)
{
    bool ret;

    ret = dpy->initialized;

    return ret;
}

void yagl_egl_display_terminate(struct yagl_egl_display *dpy)
{
    struct yagl_resource_list tmp_list;

    /*
     * First, move all resources to a temporary list to release later.
     */

    yagl_resource_list_init(&tmp_list);

    yagl_resource_list_move(&dpy->surfaces, &tmp_list);
    yagl_resource_list_move(&dpy->contexts, &tmp_list);
    yagl_resource_list_move(&dpy->configs, &tmp_list);

    dpy->initialized = false;

    yagl_resource_list_cleanup(&tmp_list);
}

int32_t yagl_egl_display_get_config_count(struct yagl_egl_display *dpy)
{
    return yagl_resource_list_get_count(&dpy->configs);
}

void yagl_egl_display_get_config_handles(struct yagl_egl_display *dpy,
                                         yagl_host_handle *handles,
                                         int32_t *num_configs)
{
    struct yagl_resource *res;
    int32_t i = 0;

    QTAILQ_FOREACH(res, &dpy->configs.resources, entry) {
        if (i >= *num_configs) {
            break;
        }
        handles[i] = res->handle;
        ++i;
    }

    *num_configs = i;
}

void yagl_egl_display_choose_configs(struct yagl_egl_display *dpy,
                                     const struct yagl_egl_native_config *dummy,
                                     yagl_host_handle *handles,
                                     int32_t *num_configs)
{
    struct yagl_resource *res;
    int32_t i = 0;

    QTAILQ_FOREACH(res, &dpy->configs.resources, entry) {
        if (handles && (i >= *num_configs)) {
            break;
        }
        if (yagl_egl_config_is_chosen_by((struct yagl_egl_config*)res, dummy)) {
            if (handles) {
                handles[i] = res->handle;
            }
            ++i;
        }
    }

    *num_configs = i;
}

struct yagl_egl_config
    *yagl_egl_display_acquire_config(struct yagl_egl_display *dpy,
                                     yagl_host_handle handle)
{
    return (struct yagl_egl_config*)yagl_resource_list_acquire(&dpy->configs, handle);
}

struct yagl_egl_config
    *yagl_egl_display_acquire_config_by_id(struct yagl_egl_display *dpy,
                                           EGLint config_id)
{
    struct yagl_resource *res;

    QTAILQ_FOREACH(res, &dpy->configs.resources, entry) {
        if (((struct yagl_egl_config*)res)->native.config_id == config_id) {
            yagl_resource_acquire(res);

            return (struct yagl_egl_config*)res;
        }
    }

    return NULL;
}

void yagl_egl_display_add_context(struct yagl_egl_display *dpy,
                                  struct yagl_egl_context *ctx)
{
    yagl_resource_list_add(&dpy->contexts, &ctx->res);
}

struct yagl_egl_context
    *yagl_egl_display_acquire_context(struct yagl_egl_display *dpy,
                                      yagl_host_handle handle)
{
    return (struct yagl_egl_context*)yagl_resource_list_acquire(&dpy->contexts, handle);
}

bool yagl_egl_display_remove_context(struct yagl_egl_display *dpy,
                                     yagl_host_handle handle)
{
    return yagl_resource_list_remove(&dpy->contexts, handle);
}

void yagl_egl_display_add_surface(struct yagl_egl_display *dpy,
                                  struct yagl_egl_surface *sfc)
{
    yagl_resource_list_add(&dpy->surfaces, &sfc->res);
}

struct yagl_egl_surface
    *yagl_egl_display_acquire_surface(struct yagl_egl_display *dpy,
                                      yagl_host_handle handle)
{
    return (struct yagl_egl_surface*)yagl_resource_list_acquire(&dpy->surfaces, handle);
}

bool yagl_egl_display_remove_surface(struct yagl_egl_display *dpy,
                                     yagl_host_handle handle)
{
    return yagl_resource_list_remove(&dpy->surfaces, handle);
}
