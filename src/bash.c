#include <pebble.h>
#include "bash.h"

Window *ready;
Window *menu;
Window *game;
Window *sequenceInput;
Window *optionsWindow;
Window *aboutWindow;

TextLayer *title;
TextLayer *start;
TextLayer *options;
TextLayer *about;
TextLayer *countdown;
TextLayer *action;
TextLayer *enterNow;
TextLayer *hs;
TextLayer *option1;
TextLayer *aboutMe;
TextLayer *aboutMe2;
TextLayer *input;
TextLayer *nextRoundOrLoseMsg;

GFont robotoB;
GFont robotoR;
GFont robotoS;
GFont robotoGame;

const int NUM_COMMANDS = 4;
const int STARTING_CAP = 10;
int capacity;
int size;
int verifIterator;
int shakeAgain;
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
    snprintf(buffer, sizeof(buffer), "Highscore\n%d", highscore);
    text_layer_set_text(hs, buffer);
  }
  score = 0;
}

static void unifiedVerifier (int action) {
  verifIterator ++;
  bool potentialSuccessfulRound = verifIterator + 1 == size;
  bool rightAction = sequence [verifIterator] == action;
  // For info purposes only
  switch (action) {
    case 0:
      APP_LOG(APP_LOG_LEVEL_INFO, "YOU PRESSED UP");
      text_layer_set_text(input, "UP");
      break;
    case 1:
      APP_LOG(APP_LOG_LEVEL_INFO, "YOU PRESSED MIDDLE");
      text_layer_set_text(input, "MIDDLE");
      break;
    case 2:
      APP_LOG(APP_LOG_LEVEL_INFO, "YOU PRESSED DOWN");
      text_layer_set_text(input, "DOWN");
      break;
    case 3:
      APP_LOG(APP_LOG_LEVEL_INFO, "YOU SHOOK THE PEBBLE");
      if (sequence [verifIterator - 1] == 3) {
        shakeAgain ++;
      } else {
        shakeAgain = 0;
      }
      if (shakeAgain > 0) {
        static char buffer[20];
        snprintf(buffer, sizeof(buffer), "shook(%d)", shakeAgain);
        text_layer_set_text(input, buffer);
      } else {
        text_layer_set_text(input, "shook");
      }
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "???");
      break;
  }
  static char buffer[30];
  if (potentialSuccessfulRound && rightAction) { // Moves onto the next round
    text_layer_set_text(input, "");
    onlyForReappearance = true; // Only when round is passed!
    layer_set_hidden((Layer*)input, true);
    layer_set_hidden((Layer*)enterNow, true);
    layer_set_hidden((Layer*)nextRoundOrLoseMsg, false);
    snprintf(buffer, sizeof(buffer), "Good job!\nNext round: %d", size + 1);
    text_layer_set_text(nextRoundOrLoseMsg, buffer);
    app_timer_register(1800, (AppTimerCallback)nextRound_callback, NULL);
    score += 10;
  } else if (!rightAction) {  // Lost the entire game
    layer_set_hidden((Layer*)input, true);
    layer_set_hidden((Layer*)enterNow, true);
    layer_set_hidden((Layer*)nextRoundOrLoseMsg, false);
    snprintf(buffer, sizeof(buffer), "Game Over.\nScore: %d", score);
    text_layer_set_text(input, "");
    text_layer_set_text(nextRoundOrLoseMsg, buffer);
    changeHighscoreIfNeeded();
    app_timer_register(2000, (AppTimerCallback)gameover_callback, NULL);
  }
}

static void veriShake_handler(AccelAxisType axis, int32_t direction) {
  unifiedVerifier (3);
}

static void veriButton_handler(ClickRecognizerRef recognizer, void *context) {
  unifiedVerifier (click_recognizer_get_button_id(recognizer) - 1);
}

static void sequence_config_provider (void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, veriButton_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, veriButton_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, veriButton_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, exit_handler);
}

static void sequenceInput_load () {
  shakeAgain = -1;
  APP_LOG(APP_LOG_LEVEL_INFO, "SequenceInput Load");
  verifIterator = -1;
  enterNow = text_layer_create(GRect(0,15,144,45));
  text_layer_set_background_color(enterNow, GColorBlack);
  text_layer_set_font(enterNow, robotoR);
  text_layer_set_text_alignment(enterNow, GTextAlignmentCenter);
  text_layer_set_text_color(enterNow, GColorWhite);
  text_layer_set_text (enterNow, "Enter sequence now!");
  
  input = text_layer_create(GRect(0,60,144,30));
  text_layer_set_background_color(input, GColorBlack);
  text_layer_set_font(input, robotoGame);
  text_layer_set_text_alignment(input, GTextAlignmentCenter);
  text_layer_set_text_color(input, GColorWhite);
  text_layer_set_text (input, "");
  
  nextRoundOrLoseMsg = text_layer_create(GRect(0,50,144,50));
  text_layer_set_background_color(nextRoundOrLoseMsg, GColorBlack);
  text_layer_set_font(nextRoundOrLoseMsg, robotoR);
  text_layer_set_text_alignment(nextRoundOrLoseMsg, GTextAlignmentCenter);
  text_layer_set_text_color(nextRoundOrLoseMsg, GColorWhite);
  text_layer_set_text (nextRoundOrLoseMsg, "");
  
  layer_set_hidden((Layer*)nextRoundOrLoseMsg, true);
  layer_add_child(window_get_root_layer(sequenceInput), text_layer_get_layer(nextRoundOrLoseMsg));
  layer_add_child(window_get_root_layer(sequenceInput), text_layer_get_layer(input));
  layer_add_child(window_get_root_layer(sequenceInput), text_layer_get_layer(enterNow));
  accel_tap_service_subscribe(veriShake_handler);
}

static void sequenceInput_unload () {
  accel_tap_service_unsubscribe();
  text_layer_destroy(input);
  text_layer_destroy(enterNow);
  text_layer_destroy(nextRoundOrLoseMsg);
  window_destroy(sequenceInput);
  APP_LOG(APP_LOG_LEVEL_INFO, "SequenceInput Unload");
}

static void sequenceInput_callback () {
  sequenceInput = window_create();
  window_set_click_config_provider(sequenceInput, sequence_config_provider);
  window_set_background_color(sequenceInput, GColorBlack);
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
    case SHAKE:
      text_layer_set_text(action, "shake!");
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
    sequence [size - 1] = rand() % NUM_COMMANDS; // 0,1,2,3
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
    if (sequence[(int)iteration] == 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "up");
    } else if (sequence [(int)iteration] == 1) {
      APP_LOG(APP_LOG_LEVEL_INFO, "middle");
    } else if (sequence [(int)iteration] == 2) {
      APP_LOG(APP_LOG_LEVEL_INFO, "down");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "shake");
    }
    timer = app_timer_register(1050, (AppTimerCallback)displaySequence, (void*)((int)iteration+1));
  } else {
    sequenceInput_callback ();
  }
}

static void game_load () {
  APP_LOG(APP_LOG_LEVEL_INFO, "Game Load");
  action = text_layer_create(GRect(0,50,144,50));
  text_layer_set_background_color(action, GColorBlack);
  text_layer_set_text_color(action, GColorWhite);
  text_layer_set_text_alignment(action, GTextAlignmentCenter);
  text_layer_set_font(action, robotoGame);
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
  window_set_background_color(game, GColorBlack);
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
  app_timer_register(1000, (AppTimerCallback)gameStart_callback, NULL);
}

static void ready_load () {
  APP_LOG(APP_LOG_LEVEL_INFO, "Ready Load");
  // output sequence.
  countdown = text_layer_create(GRect(0,60,144,50));
  text_layer_set_font(countdown, robotoGame);
  text_layer_set_text_color(countdown, GColorWhite);
  text_layer_set_background_color(countdown, GColorBlack);
  text_layer_set_text_alignment(countdown, GTextAlignmentCenter);
  text_layer_set_text(countdown, "Ready...");
  layer_add_child(window_get_root_layer(ready), text_layer_get_layer(countdown));
  app_timer_register(2000, (AppTimerCallback)changeCountdown_callback, NULL);
}

static void ready_unload () {
  text_layer_destroy(countdown);
  window_destroy (ready);
  APP_LOG(APP_LOG_LEVEL_INFO, "Ready Unload");
}

static void ready_handler (ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Clicked Start");
  ready = window_create ();
  window_set_background_color(ready, GColorBlack);
  window_set_fullscreen(ready, true);
  window_set_window_handlers(ready, (WindowHandlers) {
    .load = ready_load,
    .unload = ready_unload
  });
  window_stack_push (ready, true);
}

static void optionsWindow_load () {
  APP_LOG(APP_LOG_LEVEL_INFO, "OptionsWindow Load");
  option1 = text_layer_create(GRect(0,10,144,40));
  text_layer_set_font(option1, robotoR);
  text_layer_set_text_alignment(option1, GTextAlignmentCenter);
  text_layer_set_text_color(option1, GColorWhite);
  text_layer_set_background_color(option1, GColorBlack);
  text_layer_set_text(option1, "Reset Highscore");
  layer_add_child(window_get_root_layer(optionsWindow), text_layer_get_layer(option1));
}

static void optionsWindow_unload () {
  text_layer_destroy(option1);
  window_destroy(optionsWindow);
  APP_LOG(APP_LOG_LEVEL_INFO, "OptionsWindow unload");
}

static void optionChangeBackFromConfirmation_callback () {
  text_layer_set_text(option1, "Reset Highscore");
}

static void option1_handler(ClickRecognizerRef crr, void* context) {
  highscore = -1;
  changeHighscoreIfNeeded ();
  APP_LOG(APP_LOG_LEVEL_INFO, "Scores reset");
  text_layer_set_text(option1, "Reset Highscore\n...Done!");
  app_timer_register(900, optionChangeBackFromConfirmation_callback, NULL);
}

static void optionsWindow_click_config_provider (void *context)  {
  window_single_click_subscribe(BUTTON_ID_UP, option1_handler);
}

static void optionsWindow_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Click options");
  optionsWindow = window_create ();
  window_set_fullscreen(optionsWindow, true);
  window_set_background_color(optionsWindow, GColorBlack);
  window_set_click_config_provider(optionsWindow, optionsWindow_click_config_provider);
  window_set_window_handlers(optionsWindow, (WindowHandlers) {
    .load = optionsWindow_load,
    .unload = optionsWindow_unload
  });
  window_stack_push (optionsWindow, true);
}

static void aboutWindow_load () {
  APP_LOG(APP_LOG_LEVEL_INFO, "AboutWindow Load");
  aboutMe = text_layer_create(GRect(0,5,144,20));
  text_layer_set_text_alignment(aboutMe, GTextAlignmentCenter);
  text_layer_set_text_color(aboutMe, GColorWhite);
  text_layer_set_background_color(aboutMe, GColorBlack);
  text_layer_set_font(aboutMe, robotoR);
  text_layer_set_text(aboutMe, "bash");
  
  aboutMe2 = text_layer_create(GRect(0,30,144,138));
  text_layer_set_text_alignment(aboutMe2, GTextAlignmentCenter);
  text_layer_set_text_color(aboutMe2, GColorWhite);
  text_layer_set_background_color(aboutMe2, GColorBlack);
  text_layer_set_font(aboutMe2, robotoS);
  text_layer_set_text(aboutMe2, "born again simon with hands\n[Simon for Pebble]\n\n\n\nBy Joseph Choi\njoechoi7@gmail.com\nv0.8");
  layer_add_child(window_get_root_layer(aboutWindow), text_layer_get_layer(aboutMe));
  layer_add_child(window_get_root_layer(aboutWindow), text_layer_get_layer(aboutMe2));
}

static void aboutWindow_unload () {
  text_layer_destroy(aboutMe);
  window_destroy(aboutWindow);
  APP_LOG(APP_LOG_LEVEL_INFO, "AboutWindow Unload");
}

static void aboutWindow_handler(ClickRecognizerRef rec, void* context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Click About");
  aboutWindow = window_create();
  window_set_fullscreen(aboutWindow, true);
  window_set_background_color(aboutWindow, GColorBlack);
  window_set_window_handlers(aboutWindow, (WindowHandlers) {
    .load = aboutWindow_load,
    .unload = aboutWindow_unload
  });
  window_stack_push (aboutWindow, true);
}

static void config_provider (void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, ready_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, optionsWindow_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, aboutWindow_handler);
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
  window_set_background_color(menu, GColorBlack);
	window_stack_push(menu, true); // push onto window stack
  // Making and setting text layers
	title = text_layer_create(GRect(0, 0, 144, 40));
  start = text_layer_create(GRect(0, 45, 144, 20));
  options = text_layer_create (GRect(0, 65, 144, 20));
  about = text_layer_create (GRect(0, 85, 144, 20));
  hs = text_layer_create(GRect(0, 115, 144, 40));
  
  robotoB = fonts_load_custom_font (resource_get_handle (RESOURCE_ID_FONT_ROBOTO_BOLD_TITLE_33));
  robotoR = fonts_load_custom_font (resource_get_handle (RESOURCE_ID_FONT_ROBOTO_BOLD_TEXT_18));
  robotoS = fonts_load_custom_font (resource_get_handle (RESOURCE_ID_FONT_ROBOTO_BOLD_TEXT_14));
  robotoGame = fonts_load_custom_font (resource_get_handle (RESOURCE_ID_FONT_ROBOTO_BOLD_TEXT_28));
  
  text_layer_set_text(title, "bash");
	text_layer_set_font(title, robotoB);
	text_layer_set_text_alignment(title, GTextAlignmentCenter);
  text_layer_set_text_color(title, GColorWhite);
  text_layer_set_background_color(title, GColorBlack);
  
  text_layer_set_text(start, "Start");
	text_layer_set_font(start, robotoR);
	text_layer_set_text_alignment(start, GTextAlignmentCenter);
  text_layer_set_text_color(start, GColorWhite);
  text_layer_set_background_color(start, GColorBlack);
  
  text_layer_set_text(options, "Options");
	text_layer_set_font(options, robotoR);
	text_layer_set_text_alignment(options, GTextAlignmentCenter);
  text_layer_set_text_color(options, GColorWhite);
  text_layer_set_background_color(options, GColorBlack);
  
  text_layer_set_text(about, "About");
	text_layer_set_font(about, robotoR);
	text_layer_set_text_alignment(about, GTextAlignmentCenter);
  text_layer_set_text_color(about, GColorWhite);
  text_layer_set_background_color(about, GColorBlack);
  
  static char buffer [] = "Highscore\n999";
  snprintf(buffer, sizeof(buffer), "Highscore\n%d", highscore);
  text_layer_set_text(hs, buffer);
  text_layer_set_font(hs, robotoR);
	text_layer_set_text_alignment(hs, GTextAlignmentCenter);
  text_layer_set_text_color(hs, GColorWhite);
  text_layer_set_background_color(hs, GColorBlack);
  
  // Adding those layers to the main menu
	layer_add_child(window_get_root_layer(menu), text_layer_get_layer(title));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(start));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(options));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(about));
  layer_add_child(window_get_root_layer(menu), text_layer_get_layer(hs));
}

void handle_deinit(void) {
  persist_write_int(iKey, highscore);
  fonts_unload_custom_font(robotoR);
  fonts_unload_custom_font(robotoB);
  fonts_unload_custom_font(robotoS);
  fonts_unload_custom_font(robotoGame);
	// Destroy the text layer
	text_layer_destroy(title);
	text_layer_destroy(start);
  text_layer_destroy(about);
  text_layer_destroy(options);
  text_layer_destroy(hs);
	// Destroy the window
	window_destroy(menu);
}