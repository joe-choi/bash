#include <pebble.h>
#include <bash.h>

Window *menu;
Window *ready;
Window *game;
Window *sequenceInput;

TextLayer *title;
TextLayer *start;
TextLayer *options;
TextLayer *about;
TextLayer *countdown;
TextLayer *action;
TextLayer *enterNow;

const int NUM_COMMANDS = 3;
const int STARTING_CAP = 10;
int capacity;
int size;
int* sequence;
int timerSize;
int timerCap;
static AppTimer* timer;
bool onlyForReappearance = false;

static void delete_callback () {
  window_stack_pop(false);
}

static void sequenceInput_load () {
  APP_LOG(APP_LOG_LEVEL_INFO, "SequenceInput Load");
  enterNow = text_layer_create(GRect(0,0,144,168));
  text_layer_set_text (enterNow, "Enter commands now. Inactivity ends reading.");
  layer_add_child(window_get_root_layer(sequenceInput), text_layer_get_layer(enterNow));
  onlyForReappearance = true;
  timer = app_timer_register(1000, (AppTimerCallback)delete_callback, NULL);
}

static void sequenceInput_unload () {
  text_layer_destroy(enterNow);
  window_destroy(sequenceInput);
  APP_LOG(APP_LOG_LEVEL_INFO, "SequenceInput Unload");
}

static void sequenceInput_callback () {
  sequenceInput = window_create();
  window_set_fullscreen(sequenceInput, true);
  window_set_window_handlers(sequenceInput, (WindowHandlers) {
    .load = sequenceInput_load,
    .unload = sequenceInput_unload
  });
  window_stack_push (sequenceInput, false);
}

static void changeAction_callback (void* move) {
  switch ((int)move) {
    case UP:
      text_layer_set_text(action, "UP");
      break;
    case MIDDLE:
      text_layer_set_text(action, "MIDDLE");
      break;
    case DOWN:
      text_layer_set_text(action, "DOWN");
      break;
  }
}

static void changeAction (void* move) {
  text_layer_set_text(action, "");
  app_timer_register(100, (AppTimerCallback)changeAction_callback, (void*)move);
}

static void addElement() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Adding another element.");
  size ++;
  if (size <= capacity) {
    sequence [size - 1] = rand() % NUM_COMMANDS; // 0,1,2
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Reached capacity on ACTIONS, doubling");
    int* destroy = sequence;
    sequence = malloc (2 * (sizeof (destroy) / sizeof (int)));
    memcpy (sequence, destroy, (size - 1) * sizeof(int));
    free (destroy);
    destroy = NULL;
    capacity = capacity * 2;
  }
}

static void displaySequence(void* iteration) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Displaying sequence");
  if ((int)iteration < size) {
    changeAction ((void*)sequence[(int)iteration]);
    if (sequence[(int)iteration] == 0) { APP_LOG(APP_LOG_LEVEL_INFO, "up"); } else if (sequence [(int)iteration] == 1) { APP_LOG(APP_LOG_LEVEL_INFO, "middle");
                                                                                             } else { APP_LOG(APP_LOG_LEVEL_INFO, "down");}
    timer = app_timer_register(1050, (AppTimerCallback)displaySequence, (void*)((int)iteration+1));
  } else {
    sequenceInput_callback ();
  }
}

static void game_load () {
  APP_LOG(APP_LOG_LEVEL_INFO, "Game Load");
  action = text_layer_create(GRect(0,0,50,200));
  capacity = STARTING_CAP;
  size = 0;
  srand(time(NULL));
  if (!sequence) {
    sequence = malloc (capacity * sizeof(int));
  }
  layer_add_child(window_get_root_layer(game), text_layer_get_layer(action));
  addElement ();
  displaySequence(0);
}

static void game_unload () {
  free(sequence);
  app_timer_cancel(timer);
  text_layer_destroy(action);
  window_destroy(game);
  sequence = NULL;
  onlyForReappearance = false;
  APP_LOG(APP_LOG_LEVEL_INFO, "Game Unload");
}

static void game_appear () {
  APP_LOG(APP_LOG_LEVEL_INFO, "Game Appear function runs");
  if (onlyForReappearance) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Game Reappeared");
    text_layer_set_text(action, "");
    addElement();
    displaySequence (0);
  }
}

static void gameStart_callback () {
  game = window_create();
  window_set_fullscreen(game, true);
  window_set_window_handlers(game, (WindowHandlers) {
    .load = game_load,
    .unload = game_unload,
    .appear = game_appear
  });
  window_stack_push (game, false);
  window_stack_remove(ready, false);
}

static void changeCountdown_callback () {
  text_layer_set_text(countdown, "GO!");
}

static void ready_load () {
  APP_LOG(APP_LOG_LEVEL_INFO, "Ready Load");
  // output sequence.
  countdown = text_layer_create(GRect(0,0,50,50));
  text_layer_set_text(countdown, "Ready...");
  layer_add_child(window_get_root_layer(ready), text_layer_get_layer(countdown));
  app_timer_register(2000, (AppTimerCallback)changeCountdown_callback, NULL);
  app_timer_register(3000, (AppTimerCallback)gameStart_callback, NULL); // This will actually initiate the game window
}

static void ready_unload () {
  text_layer_destroy(countdown);
  window_destroy (ready);
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