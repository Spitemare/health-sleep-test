#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;

static void health_handler(HealthEventType event, void *context) {
    if (event == HealthEventSignificantUpdate) {
        health_handler(HealthEventSleepUpdate, context);
    } else if (event == HealthEventSleepUpdate) {
        HealthActivityMask mask = health_service_peek_current_activities();
        bool sleeping = (mask & HealthActivitySleep) || (mask & HealthActivityRestfulSleep);
        text_layer_set_text(context, sleeping ? "You are sleeping" : "You are awake");
    }
}

static void window_load(Window *window) {
    Layer *root_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root_layer);

    s_text_layer = text_layer_create(bounds);
    text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
    layer_add_child(root_layer, text_layer_get_layer(s_text_layer));

    health_handler(HealthEventSignificantUpdate, s_text_layer);
    health_service_events_subscribe(health_handler, s_text_layer);
}

static void window_unload(Window *window) {
    health_service_events_unsubscribe();
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
