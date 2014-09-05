#include <pebble.h>
#include "bash.h"

Window *ready;
Window *menu;
Window *game;
Window *sequenceInput;
Window *optionsWindow;

TextLayer *title;
TextLayer *start;
TextLayer *options;
TextLayer *about;
TextLayer *countdown;
TextLayer *action;
TextLayer *enterNow;
TextLayer *hs;
TextLayer *option1;

const int NUM_COMMANDS = 3;
const int STARTING_CAP = 10;
int capacity;
int size;
int verifIterator;
int* sequence;
static AppTimer* timer;
bool onlyForReappearance = false;
int score;
int highscore;
const int iKey = 1337;

static void exit_handler (ClickRecognizerRef recognizer, void *context) {
  window_stack_remove(game, false);
  window_stack_pop(true);
}

static void nextRound_callback () {
  window_stack_pop(false);
}

static void gameover_callback () {
  window_stack_remove(game, false);
  window_stack_pop(true);
}

static void changeHighscoreIfNeeded () {
  if (score > highscore) {
    highscore = score;
    APP_LOG(APP_LOG_LEVEL_INFO, "Changing highscore");
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "Highscore: %d", highscore);
    text_layer_set_text(hs, buffer);
  }
  score = 0;
}

static void verification_handler(ClickRecognizerRef recognizer, void *context) {
  verifIterator ++;
  bool potentialSuccessfulRound = verifIterator + 1 == size;
  int button = click_recognizer_get_button_id(recognizer) - 1;
  bool rightAction = sequence [verifIterator] == button;
  // For info purposes only
  switch (button) {
    case 0:
      APP_LOG(APP_LOG_LEVEL_INFO, "YOU PRESSED UP");
      break;
    case 1:
      APP_LOG(APP_LOG_LEVEL_INFO, "YOU PRESSED MIDDLE");
      break;
    case 2:
      APP_LOG(APP_LOG_LEVEL_INFO, "YOU PRESSED DOWN");
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "???");
      break;
  }
  static char buffer[30];
  if (potentialSuccessfulRound && rightAction) { // Moves onto the next round
    onlyForReappearance = true; // Only when round is passed!
    snprintf(buffer, sizeof(buffer), "Good job!\nNext round: %d", size + 1);
    text_layer_set_text(enterNow, buffer);
    app_timer_register(1000, (AppTimerCallback)nextRound_callback, NULL);
    score += 10;
  } else if (!rightAction) {  // Lost the entire game
    snprintf(buffer, sizeof(buffer), "Game Over.\nScore: %d", score);
    text_layer_set_text(enterNow, buffer);
    changeHighscoreIfNeeded();
    app_timer_register(1000, (AppTimerCallback)gameover_callback, NULL);
  } else if (!potentialSuccessfulRound && rightAction) {
    //score += 10;
  }
}

static void sequence_config_provider (void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, verification_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, verification_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, verification_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, exit_handler);
}

static void sequenceInput_load () {
  APP_LOG(APP_LOG_LEVEL_INFO, "SequenceInput Load");
  verifIterator = -1;
  enterNow = text_layer_create(GRect(0,0,144,168));
  text_layer_set_text (enterNow, "Enter the sequence!");
  layer_add_child(window_get_root_layer(sequenceInput), text_layer_get_layer(enterNow));
}

static void sequenceInput_unload () {
  text_layer_destroy(enterNow);
  window_destroy(sequenceInput);
  APP_LOG(APP_LOG_LEVEL_INFO, "SequenceInput Unload");
}

static void sequenceInput_callback () {
  sequenceInput = window_create();
  window_set_click_config_provider(sequenceInput, sequence_config_provider);
  //window_set_fullscreen(sequenceInput, true);
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
  timer = NULL;
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
  score = 0;
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
  if (timer != NULL) {
    app_timer_cancel(timer);
    timer = NULL;
  }
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
  //window_set_fullscreen(game, true);
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
  //window_set_fullscreen(ready, true);
  window_set_window_handlers(ready, (WindowHandlers) {
    .load = ready_load,
    .unload = ready_unload
  });
  window_stack_push (ready, true);
}

static void optionsWindow_load () {
  APP_LOG(APP_LOG_LEVEL_INFO, "OptionsWindow Load");
  option1 = text_layer_create(GRect(0,0,140,50));
  text_layer_set_text(option1, "Reset Highscore");
  layer_add_child(window_get_root_layer(optionsWindow), text_layer_get_layer(option1));
}

static void optionsWindow_unload () {
  text_layer_destroy(option1);
  window_destroy(optionsWindow);
  APP_LOG(APP_LOG_LEVEL_INFO, "OptionsWindow unload");
}

static void option1_handler(ClickRecognizerRef crr, void* context) {
  highscore = -1;
  changeHighscoreIfNeeded ();
  APP_LOG(APP_LOG_LEVEL_INFO, "Scores reset");
}

static void optionsWindow_click_config_provider (void *context)  {
  window_single_click_subscribe(BUTTON_ID_UP, option1_handler);
}

static void optionsWindow_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Click options");
  optionsWindow = window_create ();
  window_set_click_config_provider(optionsWindow, optionsWindow_click_config_provider);
  window_set_window_handlers(optionsWindow, (WindowHandlers) {
    .load = optionsWindow_load,
    .unload = optionsWindow_unload
  });
  window_stack_push (optionsWindow, true);
}

static void config_provider (void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, ready_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, optionsWindow_handler);
}

void handle_init(void) {
  // Main Menu
  if (persist_exists(iKey)) {
    highscore = persist_read_int(iKey);
  } else {
    highscore = 0;
  }
	menu = window_create(); // Creation
  window_set_click_config_provider(menu, config_provider); // Setting click config callback
  window_set_fullscreen (menu, true); // fullscreen
	window_stack_push(menu, true); // push onto window stack
  // Making and setting text layers
	title = text_layer_create(GRect(0, 0, 144, 30));
  start = text_layer_create(GRect(0, 35, 144, 30));
  options = text_layer_create (GRect(0, 65, 144, 30));
  about = text_layer_create (GRect(0, 95, 144, 30));
  hs = text_layer_create(GRect(0, 125, 144, 30));
  
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
  static char buffer [] = "Highscore: 999";
  snprintf(buffer, sizeof(buffer), "Highscore: %d", highscore);
  text_layer_set_text(hs, buffer);
  text_layer_set_font(hs, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(hs, GTextAlignmentCenter);
  // Adding those layers to the main menu
	layer_add_child(window_get_root_layer(menu), text_layer_get_layer(title));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(start));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(options));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(about));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(hs));
}

void handle_deinit(void) {
  persist_write_int(iKey, highscore);
	// Destroy the text layer
	text_layer_destroy(title);
	text_layer_destroy(start);
  text_layer_destroy(about);
  text_layer_destroy(options);
  text_layer_destroy(hs);
	// Destroy the window
	window_destroy(menu);
}