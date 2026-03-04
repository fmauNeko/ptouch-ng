/*
 * M0.8 spike: LVGL malloc strategy + SPIRAM measurement.
 * Supports both LV_USE_CLIB_MALLOC and LV_USE_CUSTOM_MALLOC paths.
 * When LV_USE_CUSTOM_MALLOC=y the five lv_*_core hooks below are required.
 * When LV_USE_CLIB_MALLOC=y LVGL provides its own wrappers — hooks must be
 * omitted to avoid duplicate symbol linker errors.
 */

#include <stddef.h>

#include "esp_heap_caps.h"
#include "esp_log.h"

#include "bsp/pandatouch.h"

static const char *TAG = "spike-malloc";

#if CONFIG_LV_USE_CUSTOM_MALLOC
void lv_mem_init(void)   { }   /* no-op: LVGL TLSF heap disabled */
void lv_mem_deinit(void) { }   /* no-op: LVGL TLSF heap disabled */

void *lv_malloc_core(size_t size)
{
    void *p = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p) {
        p = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    return p;
}

void *lv_realloc_core(void *p, size_t new_size)
{
    if (new_size == 0) {
        heap_caps_free(p);
        return NULL;
    }
    void *new_p = heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!new_p) {
        new_p = heap_caps_realloc(p, new_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    return new_p;
}

void lv_free_core(void *p)
{
    heap_caps_free(p);
}
#endif /* CONFIG_LV_USE_CUSTOM_MALLOC */

void spike_malloc_run(void)
{
    multi_heap_info_t info;

    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    size_t psram_before = info.total_free_bytes;
    ESP_LOGI(TAG, "PSRAM free before bsp_display_start: %u KB",
             (unsigned)(psram_before / 1024));

    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    size_t internal_before = info.total_free_bytes;
    ESP_LOGI(TAG, "Internal free before bsp_display_start: %u KB",
             (unsigned)(internal_before / 1024));

    bsp_display_start();
    ESP_ERROR_CHECK(bsp_display_brightness_set(80));

    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    size_t psram_after = info.total_free_bytes;
    ESP_LOGI(TAG, "PSRAM free after  bsp_display_start: %u KB",
             (unsigned)(psram_after / 1024));

    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    size_t internal_after = info.total_free_bytes;
    ESP_LOGI(TAG, "Internal free after  bsp_display_start: %u KB",
             (unsigned)(internal_after / 1024));

    size_t psram_delta    = (psram_before    > psram_after)    ? psram_before    - psram_after    : 0;
    size_t internal_delta = (internal_before > internal_after) ? internal_before - internal_after : 0;
    ESP_LOGI(TAG, "PSRAM delta: %u KB  Internal delta: %u KB",
             (unsigned)(psram_delta / 1024), (unsigned)(internal_delta / 1024));

#if defined(CONFIG_LV_USE_CLIB_MALLOC) && CONFIG_LV_USE_CLIB_MALLOC
    ESP_LOGI(TAG, "MALLOC_STRATEGY: CLIB");
    if (psram_delta > 100 * 1024) {
        ESP_LOGI(TAG, "SPIKE_RESULT: PASS — CLIB malloc, PSRAM delta %u KB",
                 (unsigned)(psram_delta / 1024));
    } else {
        ESP_LOGW(TAG, "SPIKE_RESULT: FAIL — CLIB malloc but PSRAM delta only %u KB",
                 (unsigned)(psram_delta / 1024));
    }
#else
    ESP_LOGI(TAG, "MALLOC_STRATEGY: CUSTOM_MALLOC (FALLBACK)");
    ESP_LOGI(TAG, "SPIKE_RESULT: FALLBACK — CLIB malloc unavailable, using CUSTOM_MALLOC, PSRAM delta %u KB",
             (unsigned)(psram_delta / 1024));
#endif
}
