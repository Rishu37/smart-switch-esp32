/* Multi-relay switch, based on the ESP RainMaker "switch" example
   (Public Domain / CC0 as in the original example).

   - 2, 4 or 8 relays are chosen in relay_config.h
   - each relay is its own RainMaker "Switch" device inside ONE node
   - relay states are saved to flash and restored after a power cut
   - the BOOT button / WiFi reset behaviour still comes from app_driver_init()
*/

#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_event.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_schedule.h>
#include <esp_rmaker_scenes.h>
#include <esp_rmaker_console.h>
#include <esp_rmaker_ota.h>
#include <esp_rmaker_common_events.h>

#include <app_network.h>
#include <app_insights.h>

#include "app_priv.h"
#include "relay_config.h"

static const char *TAG = "app_main";

/* The stock app_driver.c refers to this global (it reports BOOT-button presses
 * through it), so it must exist or the link step fails. We point it at relay 1. */
esp_rmaker_device_t *switch_device;

/* ---------- relay table (from relay_config.h) ---------- */
static const gpio_num_t relay_gpio[NUM_RELAYS] = RELAY_GPIO_LIST;
static const char *relay_name[NUM_RELAYS] = RELAY_NAME_LIST;

/* Bit i of this mask = relay i is ON. Saved to flash on every change. */
static uint8_t relay_mask = 0;

#define NVS_NAMESPACE "relays"
#define NVS_KEY_MASK  "mask"

static void relay_mask_load(void)
{
#if RESTORE_LAST_STATE
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) == ESP_OK) {
        uint8_t m = 0;
        if (nvs_get_u8(h, NVS_KEY_MASK, &m) == ESP_OK) {
            relay_mask = m;
        }
        nvs_close(h);
    }
#else
    relay_mask = 0;
#endif
}

static void relay_mask_save(void)
{
#if RESTORE_LAST_STATE
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_u8(h, NVS_KEY_MASK, relay_mask);
        nvs_commit(h);
        nvs_close(h);
    }
#endif
}

static inline bool relay_is_on(int idx)
{
    return (relay_mask >> idx) & 1;
}

/* Drive the pin for relay idx. Does not touch the saved mask. */
static void relay_write_pin(int idx, bool on)
{
    gpio_set_level(relay_gpio[idx], on ? RELAY_ON_LEVEL : !RELAY_ON_LEVEL);
}

static void relays_init_gpio(void)
{
    for (int i = 0; i < NUM_RELAYS; i++) {
        gpio_reset_pin(relay_gpio[i]);
        /* set the level first so the pin comes up in the right state */
        relay_write_pin(i, relay_is_on(i));
        gpio_set_direction(relay_gpio[i], GPIO_MODE_OUTPUT);
    }
}

/* Callback to handle commands received from the RainMaker cloud / app / schedules */
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
        const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    int idx = (int)(intptr_t)priv_data;
    if (idx < 0 || idx >= NUM_RELAYS) {
        return ESP_FAIL;
    }
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_POWER_NAME) == 0) {
        ESP_LOGI(TAG, "Relay %d (%s) -> %s", idx + 1, esp_rmaker_device_get_name(device),
                 val.val.b ? "ON" : "OFF");
        if (val.val.b) {
            relay_mask |= (1u << idx);
        } else {
            relay_mask &= ~(1u << idx);
        }
        relay_write_pin(idx, val.val.b);
        relay_mask_save();
        esp_rmaker_param_update(param, val);
    }
    return ESP_OK;
}

/* Event handler for catching RainMaker events */
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == RMAKER_EVENT) {
        switch (event_id) {
            case RMAKER_EVENT_INIT_DONE:
                ESP_LOGI(TAG, "RainMaker Initialised.");
                break;
            case RMAKER_EVENT_CLAIM_STARTED:
                ESP_LOGI(TAG, "RainMaker Claim Started.");
                break;
            case RMAKER_EVENT_CLAIM_SUCCESSFUL:
                ESP_LOGI(TAG, "RainMaker Claim Successful.");
                break;
            case RMAKER_EVENT_CLAIM_FAILED:
                ESP_LOGI(TAG, "RainMaker Claim Failed.");
                break;
            case RMAKER_EVENT_LOCAL_CTRL_STARTED:
                ESP_LOGI(TAG, "Local Control Started.");
                break;
            case RMAKER_EVENT_LOCAL_CTRL_STOPPED:
                ESP_LOGI(TAG, "Local Control Stopped.");
                break;
            default:
                ESP_LOGW(TAG, "Unhandled RainMaker Event: %"PRIi32, event_id);
        }
    } else if (event_base == RMAKER_COMMON_EVENT) {
        switch (event_id) {
            case RMAKER_EVENT_REBOOT:
                ESP_LOGI(TAG, "Rebooting in %d seconds.", *((uint8_t *)event_data));
                break;
            case RMAKER_EVENT_WIFI_RESET:
                ESP_LOGI(TAG, "Wi-Fi credentials reset.");
                break;
            case RMAKER_EVENT_FACTORY_RESET:
                ESP_LOGI(TAG, "Node reset to factory defaults.");
                /* a factory reset should also forget saved relay states */
                relay_mask = 0;
                relay_mask_save();
                break;
            case RMAKER_MQTT_EVENT_CONNECTED:
                ESP_LOGI(TAG, "MQTT Connected.");
                break;
            case RMAKER_MQTT_EVENT_DISCONNECTED:
                ESP_LOGI(TAG, "MQTT Disconnected.");
                break;
            case RMAKER_MQTT_EVENT_PUBLISHED:
                ESP_LOGI(TAG, "MQTT Published. Msg id: %d.", *((int *)event_data));
                break;
            default:
                ESP_LOGW(TAG, "Unhandled RainMaker Common Event: %"PRIi32, event_id);
        }
    } else if (event_base == APP_NETWORK_EVENT) {
        switch (event_id) {
            case APP_NETWORK_EVENT_QR_DISPLAY:
                ESP_LOGI(TAG, "Provisioning QR : %s", (char *)event_data);
                break;
            case APP_NETWORK_EVENT_PROV_TIMEOUT:
                ESP_LOGI(TAG, "Provisioning Timed Out. Please reboot.");
                break;
            case APP_NETWORK_EVENT_PROV_RESTART:
                ESP_LOGI(TAG, "Provisioning has restarted due to failures.");
                break;
            default:
                ESP_LOGW(TAG, "Unhandled App Wi-Fi Event: %"PRIi32, event_id);
                break;
        }
    } else if (event_base == RMAKER_OTA_EVENT) {
        switch (event_id) {
            case RMAKER_OTA_EVENT_STARTING:
                ESP_LOGI(TAG, "Starting OTA.");
                break;
            case RMAKER_OTA_EVENT_IN_PROGRESS:
                ESP_LOGI(TAG, "OTA is in progress.");
                break;
            case RMAKER_OTA_EVENT_SUCCESSFUL:
                ESP_LOGI(TAG, "OTA successful.");
                break;
            case RMAKER_OTA_EVENT_FAILED:
                ESP_LOGI(TAG, "OTA Failed.");
                break;
            case RMAKER_OTA_EVENT_REJECTED:
                ESP_LOGI(TAG, "OTA Rejected.");
                break;
            case RMAKER_OTA_EVENT_DELAYED:
                ESP_LOGI(TAG, "OTA Delayed.");
                break;
            case RMAKER_OTA_EVENT_REQ_FOR_REBOOT:
                ESP_LOGI(TAG, "Firmware image downloaded. Please reboot your device to apply the upgrade.");
                break;
            default:
                ESP_LOGW(TAG, "Unhandled OTA Event: %"PRIi32, event_id);
                break;
        }
    } else {
        ESP_LOGW(TAG, "Invalid event received!");
    }
}

void app_main()
{
    /* Console + the stock driver (this gives us the BOOT button: 3 s = WiFi reset,
     * 10 s = factory reset). We do NOT use its single output pin. */
    esp_rmaker_console_init();
    app_driver_init();

    /* Initialize NVS. */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /* Restore the last relay states and set the pins (NVS must be ready first). */
    relay_mask_load();
    relays_init_gpio();
    ESP_LOGI(TAG, "Variant: %d relays, restored mask = 0x%02x", NUM_RELAYS, relay_mask);

    /* Initialize Wi-Fi. Must be called before esp_rmaker_node_init() */
    app_network_init();

    /* Register an event handler to catch RainMaker events */
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_COMMON_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(APP_NETWORK_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_OTA_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    /* Initialize the ESP RainMaker Agent. */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "Automated Switch", "Switch");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

    /* One RainMaker Switch device per relay, all inside this one node. */
    for (int i = 0; i < NUM_RELAYS; i++) {
        esp_rmaker_device_t *dev = esp_rmaker_device_create(relay_name[i],
                ESP_RMAKER_DEVICE_SWITCH, (void *)(intptr_t)i);
        esp_rmaker_device_add_cb(dev, write_cb, NULL);
        esp_rmaker_device_add_param(dev,
                esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, relay_name[i]));
        esp_rmaker_param_t *power_param =
                esp_rmaker_power_param_create(ESP_RMAKER_DEF_POWER_NAME, relay_is_on(i));
        esp_rmaker_device_add_param(dev, power_param);
        esp_rmaker_device_assign_primary_param(dev, power_param);
        esp_rmaker_node_add_device(node, dev);
        if (i == 0) {
            switch_device = dev;   /* needed by app_driver.c, see top of file */
        }
    }

    /* Enable OTA */
    esp_rmaker_ota_enable_default();

    /* Timezone service (needed so schedules run at the right local time). */
    esp_rmaker_timezone_service_enable();

    /* Enable scheduling. */
    esp_rmaker_schedule_enable();

    /* Enable Scenes */
    esp_rmaker_scenes_enable();

    /* Enable Insights. Requires CONFIG_ESP_INSIGHTS_ENABLED=y */
    app_insights_enable();

    /* Start the ESP RainMaker Agent */
    esp_rmaker_start();

    err = app_network_set_custom_mfg_data(MFG_DATA_DEVICE_TYPE_SWITCH, MFG_DATA_DEVICE_SUBTYPE_SWITCH);

    /* Start the Wi-Fi. If provisioned it connects, else it starts provisioning. */
    err = app_network_start(POP_TYPE_RANDOM);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }
}
