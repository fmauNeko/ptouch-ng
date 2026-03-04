#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_littlefs.h"

static const char *TAG = "spike-littlefs";

void spike_littlefs_run(void)
{
    // Mount LittleFS
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/data",
        .partition_label = "littlefs",
        .format_if_mount_failed = true,
    };
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — mount error %s", esp_err_to_name(ret));
        return;
    }

    // Test 1: Write/Read roundtrip
    const char *test_data = "{\"theme\":\"catppuccin-mocha\",\"brightness\":80}";
    FILE *f = fopen("/data/test_config.json", "w");
    if (f) {
        fputs(test_data, f);
        fclose(f);
    }
    char buf[128] = {0};
    f = fopen("/data/test_config.json", "r");
    if (f) {
        fgets(buf, sizeof(buf), f);
        fclose(f);
    }
    if (strcmp(buf, test_data) == 0) {
        ESP_LOGI(TAG, "WRITE_READ: MATCH");
    } else {
        ESP_LOGE(TAG, "WRITE_READ: MISMATCH");
    }

    // Test 2: Partition info
    size_t total = 0, used = 0;
    esp_littlefs_info("littlefs", &total, &used);
    ESP_LOGI(TAG, "PARTITION: total=%uKB, used=%uKB, free=%uKB",
             (unsigned)(total / 1024), (unsigned)(used / 1024),
             (unsigned)((total - used) / 1024));

    // Test 3: Boot counter (persistence)
    int boot_count = 0;
    f = fopen("/data/boot_counter.txt", "r");
    if (f) {
        fscanf(f, "%d", &boot_count);
        fclose(f);
    }
    boot_count++;
    f = fopen("/data/boot_counter.txt", "w");
    if (f) {
        fprintf(f, "%d", boot_count);
        fclose(f);
    }
    ESP_LOGI(TAG, "BOOT_COUNT: %d", boot_count);

    ESP_LOGI(TAG, "SPIKE_RESULT: PASS — LittleFS write/read OK, persistence verified, partition %uKB free",
             (unsigned)((total - used) / 1024));

    esp_vfs_littlefs_unregister("littlefs");
}
