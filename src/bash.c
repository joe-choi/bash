#include <pebble.h>

Window *menu;
Window *ready;
Window *game;

TextLayer *title;
TextLayer *start;
TextLayer *options;
TextLayer *about;
TextLayer *countdown;

static void game_load () {
  APP_LOG(APP_LOG_LEVEL_INFO, "Game Load");
}

static void game_unload () {
  window_destroy(game);
  APP_LOG(APP_LOG_LEVEL_INFO, "Game Unload");
}

static void gameStart_callback () {
  game = window_create();
  window_set_fullscreen(game, true);
  window_set_window_handlers(game, (WindowHandlers) {
    .load = game_load,
    .unload = game_unload
  });
  window_stack_push (game, false);
  window_stack_remove(ready, false);
}

static void changeCountdown_callback () {
  text_layer_set_text(countdown, "GO!");
}

static void ready_load (Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Ready Load");
  // output sequence.
  countdown = text_layer_create(GRect(0,0,50,50));
  text_layer_set_text(countdown, "Ready...");
  layer_add_child(window_get_root_layer(ready), text_layer_get_layer(countdown));
  app_timer_register(2000, (AppTimerCallback)changeCountdown_callback, NULL);
  app_timer_register(3000, (AppTimerCallback)gameStart_callback, NULL); // This will actually initiate the game window
}

static void ready_unload (Window *window) {
  text_layer_destroy(countdown);
  window_destroy (window);
  APP_LOG(APP_LOG_LEVEL_INFO, "Ready Unload");
}

static void ready_handler (ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Clicked Start");
  ready = window_create ();
  window_set_fullscreen(ready, true);
  window_set_window_handlers(ready, (WindowHandlers) {
    .load = ready_load,
    .unload = ready_unload
  });
  window_stack_push (ready, true);
}

static void config_provider (void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, ready_handler);
}

void handle_init(void) {
  // Main Menu
	menu = window_create(); // Creation
  window_set_click_config_provider(menu, config_provider); // Setting click config callback
  window_set_fullscreen (menu, true); // fullscreen
	window_stack_push(menu, true); // push onto window stack
  // Making and setting text layers
	title = text_layer_create(GRect(0, 0, 144, 30));
  start = text_layer_create(GRect(0, 45, 144, 30));
  options = text_layer_create (GRect(0, 75, 144, 30));
  about = text_layer_create (GRect(0, 105, 144, 30));
  text_layer_set_text(title, "bash"); 
	text_layer_set_font(title, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(title, GTextAlignmentCenter);
  text_layer_set_text(start, "Start");
	text_layer_set_font(start, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(start, GTextAlignmentCenter);
  text_layer_set_text(options, "Options");
	text_layer_set_font(options, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(options, GTextAlignmentCenter);
  text_layer_set_text(about, "About");
	text_layer_set_font(about, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(about, GTextAlignmentCenter);
  // Adding those layers to the main menu
	layer_add_child(window_get_root_layer(menu), text_layer_get_layer(title));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(start));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(options));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(about));
}

void handle_deinit(void) {
	// Destroy the text layer
	text_layer_destroy(title);
	text_layer_destroy(start);
  text_layer_destroy(about);
  text_layer_destroy(options);
	// Destroy the window
	window_destroy(menu);
}