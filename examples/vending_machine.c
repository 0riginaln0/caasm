#include <stdio.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define AASM_IMPLEMENTATION
#include "../caasm.h"


// https://www.nationin.com/post/how-to-implement-a-sequence-detector-using-mealy-finite-state-machine-in-digital-electronics

enum Vending_State {
  STATE_IDLE,
  STATE_COIN_INSERTED,
  STATE_PRODUCT_SELECTED,
  STATE_DISPENSE_PRODUCT,
  STATE_OUT_OF_STOCK,
};

enum Vending_Event {
  EVENT_COIN_INSERTED,
  EVENT_PRODUCT_BUTTON_PRESSED,
  EVENT_CHANGE_RETURNED,
  EVENT_OUT_OF_STOCK_SIGNAL,
};


void display_select_product(void *ctx) {
  // get list of products and their values from ctx
  // printf them
}

void display_confirm_product(void *ctx) {
  // get price and name of the product from ctx
  // Confirm (show price) printf
}

void display_message(void *ctx) {
  printf("MESSAGE: %s\n", "aboba");
}

void return_money_and_display_out_of_stock(void *ctx) {

}

void display_out_of_stock(void *ctx) {

}

int dispense_product(void *ctx) {
  // also print message if they want change returned
  return 99;
}

int return_change(void *ctx) {
  return 5;
}

bool is_enough_money(void *ctx) {
  // check that enough money insterted to buy the product.
  // display message 'insert more coins' if not enough
  // return true or false correspondingly
}

static const AASM_Event events[] = {
  AASM_EVENT(EVENT_COIN_INSERTED,
    { .from = STATE_IDLE, .to = STATE_COIN_INSERTED, .action = display_select_product },
    
    { .from = STATE_PRODUCT_SELECTED, .to = STATE_DISPENSE_PRODUCT, 
      .guard = is_enough_money, .action = dispense_product },

    { .from = STATE_OUT_OF_STOCK, .to = STATE_OUT_OF_STOCK, .action = return_money_and_display_out_of_stock },
  ),

  AASM_EVENT(EVENT_PRODUCT_BUTTON_PRESSED,
    { .from = STATE_COIN_INSERTED, .to = STATE_PRODUCT_SELECTED, 
      .action = display_confirm_product },
  ),

  AASM_EVENT(EVENT_CHANGE_RETURNED,
    { .from = STATE_DISPENSE_PRODUCT, .to = STATE_IDLE }
  ),

  AASM_EVENT(EVENT_OUT_OF_STOCK_SIGNAL,
    { .from = STATE_IDLE,             .to = STATE_OUT_OF_STOCK, .action = display_out_of_stock },
    { .from = STATE_COIN_INSERTED,    .to = STATE_OUT_OF_STOCK, .action = display_out_of_stock },
    { .from = STATE_PRODUCT_SELECTED, .to = STATE_OUT_OF_STOCK, .action = display_out_of_stock },
    { .from = STATE_DISPENSE_PRODUCT, .to = STATE_OUT_OF_STOCK, .action = display_out_of_stock },
  )
};

static const AASM_Description description = {
  .initial_state = STATE_IDLE,
  .events = events,
  .events_count = ARRAY_LEN(events),
};

typedef struct {
  const char* message;
  uint16_t coin_denomination;
} App_State;

int main(void) {
  printf("Vending machine: demo started!\n");

  App_State ctx;

  AASM_Runtime runtime = {0};
  aasm_init(&runtime, &description, &ctx);

  printf("Vending machine: demo ended!\n");
}