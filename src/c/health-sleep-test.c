#include <pebble.h>
#include <pebble-console-layer/console-layer.h>

const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

static Window *s_window;
static TextLayer *s_text_layer;
static Layer *s_console_layer;

static void health_handler(HealthEventType event, void *context) {
    if (event == HealthEventSignificantUpdate) {
        health_handler(HealthEventSleepUpdate, context);
    } else if (event != HealthEventHeartRateUpdate) {
        time_t now = time(NULL);
        struct tm *tick_time = localtime(&now);
        char stime[16];
        strftime(stime, sizeof(stime), "%m-%d %H:%I:%S", tick_time);

        HealthActivityMask mask = health_service_peek_current_activities();
        char s[32];
        snprintf(s, sizeof(s), "%s_%d_%s", stime, event, bit_rep[mask & 0x0F]);
        console_layer_write_text(s_console_layer, s);
        if (event == HealthEventSleepUpdate) {
            bool sleeping = (mask & HealthActivitySleep) || (mask & HealthActivityRestfulSleep);
            text_layer_set_text(context, sleeping ? "Asleep" : "Awake");
        }
    }
}

static void window_load(Window *window) {
    Layer *root_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root_layer);

    s_text_layer = text_layer_create(bounds);
    text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(root_layer, text_layer_get_layer(s_text_layer));

    s_console_layer = console_layer_create(GRect(bounds.origin.x, bounds.origin.y + 20, bounds.size.w, bounds.size.h - 20));
    layer_add_child(root_layer, s_console_layer);

    health_handler(HealthEventSignificantUpdate, s_text_layer);
    health_service_events_subscribe(health_handler, s_text_layer);
}

static void window_unload(Window *window) {
    health_service_events_unsubscribe();
    console_layer_destroy(s_console_layer);
    text_layer_destroy(s_text_layer);
}

static void init(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });
    window_stack_push(s_window, true);
}

static void deinit(void) {
    window_destroy(s_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
