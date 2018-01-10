/**
 * \file
 * \brief Implementation of push button debouncing.
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements the debouncing of keys, using a state machine.
 * While this is a rather complex way of debouncing, it allows us to
 * detect both short and long key presses, which is a plus.
 */

#include "Platform.h"
#if PL_CONFIG_HAS_DEBOUNCE
#include <stddef.h> /* for NULL */
#include "Debounce.h"
#include "Trigger.h"

/*! \brief Key scan routine which implements the state machine.
\dot
digraph example_api_graph {
  node [shape=box];
  DBNC_KEY_IDLE    [fillcolor=lightblue,style=filled,label="DBNC_KEY_IDLE" ];
  DBNC_KEY_PRESSED [fillcolor=lightblue,style=filled,label="DBNC_KEY_PRESSED" ];
  DBNC_KEY_RELEASE [fillcolor=lightblue,style=filled,label="DBNC_KEY_WAIT_RELEASE"];
  DBNC_KEY_IDLE -> DBNC_KEY_PRESSED -> DBNC_KEY_RELEASE ;
  DBNC_KEY_PRESSED -> DBNC_KEY_PRESSED ;
  DBNC_KEY_IDLE -> DBNC_KEY_IDLE ;
}
\enddot
*/

void DBNC_Process(DBNC_FSMData *data) {
  DBNC_KeySet keys;

  for(;;) { /* for loop, will return */
    switch(data->state) {
      case DBNC_KEY_IDLE: /* idle, and now getting a key */
        data->scanValue = data->getKeys();
        data->longKeyCnt = 0;
        data->onDebounceEvent(DBNC_EVENT_PRESSED, data->scanValue); /* we have a key press: call event handler  */
        data->lastEvent = DBNC_EVENT_PRESSED;
        data->state = DBNC_KEY_PRESSED; /* advance to next state */
        (void)TRG_SetTrigger(data->trigger, data->debounceTicks, (TRG_Callback)DBNC_Process, (void*)data);
        return;
  
      case DBNC_KEY_PRESSED:
        keys = data->getKeys();
        if (keys==data->scanValue) { /* still pressing the same keys */
          if (data->longKeyCnt>=data->longKeyTicks && data->longKeyCnt<data->longlongKeyTicks && data->lastEvent==DBNC_EVENT_PRESSED) {
            data->onDebounceEvent(DBNC_EVENT_LONG_PRESSED, data->scanValue);
            data->lastEvent = DBNC_EVENT_LONG_PRESSED;
          } else if (data->longKeyCnt>=data->longlongKeyTicks && data->lastEvent==DBNC_EVENT_LONG_PRESSED) {
            data->onDebounceEvent(DBNC_EVENT_LLONG_PRESSED, data->scanValue);
            data->lastEvent = DBNC_EVENT_LLONG_PRESSED;
          } else {
            data->longKeyCnt += data->debounceTicks; /* increment loop counter */
          }
          (void)TRG_SetTrigger(data->trigger, data->debounceTicks, (TRG_Callback)DBNC_Process, (void*)data); /* continue waiting */
          return;
        } else if (keys==0) { /* all keys are released */
          data->state = DBNC_KEY_RELEASE; /* advance to next state */
          (void)TRG_SetTrigger(data->trigger, data->debounceTicks, (TRG_Callback)DBNC_Process, (void*)data);
          return;
        } else { /* we got another key set pressed */
          data->onDebounceEvent(DBNC_EVENT_PRESSED, keys & ~(data->scanValue)); /* new pressed key */
          data->state = DBNC_KEY_RELEASE;
        }
        break;
  
      case DBNC_KEY_RELEASE: /* wait until keys are released */
        keys = data->getKeys();
        if (keys==0) { /* all keys released, go back to idle state. */
          if (data->lastEvent == DBNC_EVENT_LLONG_PRESSED) {
            data->onDebounceEvent(DBNC_EVENT_LLONG_RELEASED, data->scanValue);
          } else if (data->lastEvent == DBNC_EVENT_LONG_PRESSED) {
            data->onDebounceEvent(DBNC_EVENT_LONG_RELEASED, data->scanValue);
          } else {
            data->onDebounceEvent(DBNC_EVENT_RELEASED, data->scanValue);
          }
          data->state = DBNC_KEY_IDLE; /* go back to idle */
          data->onDebounceEvent(DBNC_EVENT_END, data->scanValue); /* callback at the end of debouncing. */
          data->lastEvent = DBNC_EVENT_END;
          return;
        } else { /* continue waiting */
          /* create events for the delta */
          data->onDebounceEvent(DBNC_EVENT_RELEASED, (uint8_t)(data->scanValue&(~keys)));
          data->lastEvent = DBNC_EVENT_RELEASED;
          data->scanValue = keys;
          data->longKeyCnt = 0; /* star counting again */
          data->state = DBNC_KEY_PRESSED;
        }
        break;
    } /* switch */
  } /* for */
}

void DBNC_Deinit(void) {
  /* nothing for now */
}

void DBNC_Init(void) {
  /* nothing to init for now */
}
#endif /* PL_CONFIG_HAS_DEBOUNCE */
