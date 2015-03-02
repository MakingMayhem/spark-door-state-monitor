// door-state-monitor.ino
//
////////////////////////////////////////////////////////////////////////////////
//
// The MIT License (MIT)
//
// Copyright (c) 2015 David L Kinney <david@pinkhop.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////
//
// This project combines a Spark device with a reed switch ("door sensor") to
// publish an event when a door has been left open. The event can be detected
// by another service -- such as IFTTT -- and used to trigger actions, such as
// sending an SMS notification.
//
// Whenever the door is opened or closed, a Spark event named
// "door-state-changed" is published with a payload of "open" or "closed",
// respectively. When the door remains open more than a specified period of
// time (default is 10 seconds), a "door-state-changed" event is published with
// a payload of "openalert".
//
// The current state of the door can be queried using the "state" variable.
//
// Certain application behaviors can be controlled by macros. Check the
// "USER-CONTROLLED SETTINGS" section, below, for more information.
//


////////////////////////////////////////////////////////////////////////////////
// USER-CONTROLLED SETTINGS

// Pin on which to sense the reed switch ("door sensor")
#define DOOR_PIN  D0

// Number of seconds the door must be open before the an alert is triggered
#define OPEN_ALERT_SECONDS  10

// For debugging and to assist setup of the door sensor, the Spark's built-in
// LED will display the value of DOOR_PIN.
//
// To disable this feature, leaving the LED off, comment out the following
// line.
#define LED_PIN  D7

// When the openalert state is triggered, the Spark's RGB LED will turn solid
// orange. It will resume normal behavior once the door is closed.
//
// To disable this feature, comment out the following line or set value to 0.
#define USE_RGB 1


////////////////////////////////////////////////////////////////////////////////
// MACROS

#define ONE_SECOND_MILLIS        (1000)
#define ONE_MINUTE_MILLIS        (60 * ONE_SECOND_MILLIS)
#define ONE_HOUR_MILLIS          (60 * ONE_MINUTE_MILLIS)
#define ONE_DAY_MILLIS           (24 * ONE_HOUR_MILLIS)
#define DOOR_CLOSED_PIN_VALUE    LOW
#define DOOR_OPEN_PIN_VALUE      HIGH
#define STATE_NAME_BUFFER_SIZE   10
#define EVENT_NAME               "door-state-changed"
#define EVENT_TTL                30
#define STATE_NAME_CLOSED        "closed"
#define STATE_NAME_OPEN          "open"
#define STATE_NAME_OPEN_ALERT    "openalert"

#define OPEN_ALERT_MILLIS        (OPEN_ALERT_SECONDS * ONE_SECOND_MILLIS)


////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS

typedef enum DoorState {
    DoorStateClosed,
    DoorStateOpen,
    DoorStateOpenAlert,
} DoorState;


////////////////////////////////////////////////////////////////////////////////
// VARIABLES

unsigned long lastSyncedAt;
unsigned long doorOpenedAt;
DoorState state;
char stateName[STATE_NAME_BUFFER_SIZE];


void setStateName(String name) {
    name.toCharArray(stateName, STATE_NAME_BUFFER_SIZE);
}


////////////////////////////////////////////////////////////////////////////////
// STATE TRANSITIONS

void transitionToStateClosed() {
    state = DoorStateClosed;
    doorOpenedAt = 0;
    setStateName(STATE_NAME_CLOSED);
    Spark.publish(EVENT_NAME, STATE_NAME_CLOSED, EVENT_TTL, PRIVATE);

}

void transitionToStateClosedFromAlert() {
    // Resume normal RGB LED operation
#if USE_RGB
    RGB.control(false);
#endif

    // Continue with normal transition to Closed
    transitionToStateClosed();
}

void transitionToStateOpen() {
    state = DoorStateOpen;
    doorOpenedAt = millis();
    setStateName(STATE_NAME_OPEN);
    Spark.publish(EVENT_NAME, STATE_NAME_OPEN, EVENT_TTL, PRIVATE);

}

void transitionToStateOpenAlert() {
    state = DoorStateOpenAlert;

    // Turn RBG LED orange
#if USE_RGB
    RGB.control(true);
    RGB.color(255, 165, 0); // orange
#endif

    // doorOpenedAt is not reset
    setStateName(STATE_NAME_OPEN_ALERT);
    Spark.publish(EVENT_NAME, STATE_NAME_OPEN_ALERT, EVENT_TTL, PRIVATE);
}


////////////////////////////////////////////////////////////////////////////////
// MAIN PROGRAM

void setup() {
    transitionToStateClosed();

    Spark.variable("state", stateName, STRING);

    pinMode(DOOR_PIN, INPUT_PULLUP);
#ifdef LED_PIN
    pinMode(LED_PIN, OUTPUT);
#endif
}

void loop() {
    unsigned long const now = millis();

    int const doorPinValue = digitalRead(DOOR_PIN);
#ifdef LED_PIN
    digitalWrite(LED_PIN, doorPinValue);
#endif

    if (DoorStateClosed == state) {
        if (DOOR_CLOSED_PIN_VALUE == doorPinValue) {
            // do nothing
        } else if (DOOR_OPEN_PIN_VALUE == doorPinValue) {
            transitionToStateOpen();
        }
    } else if (DoorStateOpen == state) {
        if (DOOR_CLOSED_PIN_VALUE == doorPinValue) {
            transitionToStateClosed();
        } else if (DOOR_OPEN_PIN_VALUE == doorPinValue) {
            unsigned long const millisOpen = now - doorOpenedAt;
            if (millisOpen > OPEN_ALERT_MILLIS) {
                transitionToStateOpenAlert();
            }
        }
    } else if (DoorStateOpenAlert == state) {
        if (DOOR_CLOSED_PIN_VALUE == doorPinValue) {
            transitionToStateClosedFromAlert();
        }
    }

    // Prevent the Spark's date/time from straying too much from real date/time by
    // synchronizing it once a day.
    if (now - lastSyncedAt > ONE_DAY_MILLIS) {
        // Request time synchronization from the Spark Cloud
        Spark.syncTime();
        lastSyncedAt = now;
    }
}
