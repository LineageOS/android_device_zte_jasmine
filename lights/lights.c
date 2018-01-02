/*
 * Copyright (C) 2016 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "lights"

#include <cutils/log.h>

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>

#define UNUSED __attribute__((unused))

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static const char* const lcd_backlight_filename =
    "/sys/class/leds/lcd-backlight/brightness";

static void
write_file(const char *path, const char *str)
{
    int fd;
    ssize_t written = -1;

    ALOGI("%s: path=%s str=%s\n", __func__, path, str);
    fd = open(path, O_RDWR);
    if (fd < 0) {
        ALOGE("%s: Failed to open %s: %s\n",
                __func__, path, strerror(errno));
        return;
    }
    written = write(fd, str, strlen(str));
    if (written < 0) {
        ALOGE("%s: Failed to write to %s: %s\n",
                __func__, path, strerror(errno));
    }
    close(fd);
}

static uint32_t
rgb_to_brightness(const struct light_state_t *state)
{
    uint32_t color = state->color & 0x00ffffff;
    return ((77 * ((color >> 16) & 0xff)) +
            (150 * ((color >> 8) & 0xff)) +
            (29 * (color & 0xff))) >> 8;
}

static void
init_globals(void)
{
    pthread_mutex_init(&g_lock, NULL);
}

static int
set_light_backlight(UNUSED struct light_device_t *dev,
        const struct light_state_t *state)
{
    char val[12+1];

    snprintf(val, sizeof(val), "%u", rgb_to_brightness(state));

    pthread_mutex_lock(&g_lock);

    write_file(lcd_backlight_filename, val);

    pthread_mutex_unlock(&g_lock);

    return 0;
}

static int
close_lights(struct light_device_t *dev)
{
    free(dev);
    return 0;
}

static int
open_lights(const struct hw_module_t *module, const char *name,
        struct hw_device_t **device)
{
    int (*set_light)(struct light_device_t *dev,
            const struct light_state_t *state);
    struct light_device_t *dev;

    if (0 == strcmp(LIGHT_ID_BACKLIGHT, name)) {
        set_light = set_light_backlight;
    } else {
        return -EINVAL;
    }

    pthread_once(&g_init, init_globals);
    dev = malloc(sizeof(struct light_device_t));
    memset(dev, 0, sizeof(struct light_device_t));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t *) module;
    dev->common.close = (int (*)(struct hw_device_t *)) close_lights;
    dev->set_light = set_light;

    *device = (struct hw_device_t *)dev;
    return 0;

}

static struct hw_module_methods_t lights_module_methods = {
    .open = open_lights,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "Lights module",
    .author = "The CyanogenMod Project",
    .methods = &lights_module_methods,
};
