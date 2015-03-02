# Door State Monitor

This project combines a Spark device with a reed switch ("door sensor") to publish an event when a door has been left open. The event can be detected by another service -- such as IFTTT -- and used to trigger actions, such as sending an SMS notification.

Whenever the door is opened or closed, a Spark event named "door-state-changed" is published with a payload of "open" or "closed", respectively. When the door remains open more than a specified period of time (default is 10 seconds), a "door-state-changed" event is published with a payload of "openalert".

The current state of the door can be queried using the "state" variable.


## Getting Started

This document assumes that you are familiar with the Spark platform's hardware and development environment. Otherwise, visit the Spark [Getting Started Guide][GettingStarted] before continuing.

### Hardware

#### Bill of Materials

This project requires:

- A [Spark Core][Core] or [Spark Photon][Photon], available for purchase from [Spark.io][Spark@Spark] or [Adafruit][Spark@Adafruit]
- A reed switch ("door sensor"), available for purchase from [Adafruit][ReedSensor@Adafruit]

#### Setup

1. Connect the switch to a digital pin, such as `D0`, on the Spark device, as shown in the diagram below
2. Connect the Spark device to power and ground; for example,  connect its USB port to a USB charger

![Fritzing diagram showing a reed switch connected to GND and D0 on the Spark](./docs/images/spark-door-state-monitor-400x336.png)

### Software

These instructions assume you are using the [Spark Build][SparkBuild] Web IDE.

1. Click the Code button on the left vertical ribbon
2. Click the "Create New App" button
3. Name the app "Door-State-Monitor"
4. Copy and paste the content from `door-state-monitor.ino` into the editor
5. Click the "Save" button on the left vertical ribbon

Now you can flash it to your Spark devices normally.

#### Configuration

Find the `USER-CONTROLLED SETTINGS` section of the source code, starting around line 47.

If you want to hook up the door sensor to a different digital IO pin, change value of the `DOOR_PIN` macro.

If you want to change the number of seconds the door is open before the alert mode is triggered, change the value of the `OPEN_ALERT_SECONDS` macro.

If you do not want the `D7` LED to light up when the door is open, comment out the `LED_PIN` macro.

If you do not want the RGB LED to turn orange when the open alert mode is triggered, set the `USE_RGB` macro to `0`.


## Monitoring

### cURL

You can query the state of the door using the exported `state` variable (replace `ACCESS_TOKEN` and `DEVICE_ID` appropriately):

    curl \
      --header 'Authorization: Bearer ACCESS_TOKEN' \
      https://api.spark.io/v1/devices/DEVICE_ID/state

You can subscribe to state changes of the door as well:

    curl \
      --header 'Authorization: Bearer ACCESS_TOKEN' \
      https://api.spark.io/v1/events/door-state-change

### IFTTT

This application gets interesting when a service is monitoring the door state and able to take action. A great service for monitoring Spark events is [IFTTT][IFTTT]. Spark provides an excellent tutorial for [learning how to get started with Spark and IFTTT][SparkIFTTT].


## Next Steps

This section enumerates my ideas for enhancements to the project.

### IFTTT Example

Include a document that walks through setting up SMS notifications when the door is left open.

### Battery Power Mode

On the Spark Core, disable WiFi until the door is opened. In this mode, the Core uses 30-38 mA of current.

On the Spark Photon, enter deep sleep mode until the door is opened via the `Spark.sleep(D0, RISING)` command. In this mode, the Photon uses 3.2 μA.


## License

This project is available under the terms of the MIT License. See the `LICENSE` file for more information.



[Core]: https://store.spark.io/?product=spark-core
[GettingStarted]: http://docs.spark.io/start/
[IFTTT]: https://ifttt.com
[Photon]: https://store.spark.io/?product=spark-photon
[ReedSensor@Adafruit]: https://www.adafruit.com/product/375
[SparkBuild]: http://docs.spark.io/build/
[SparkIFTTT]: http://docs.spark.io/ifttt/
[Spark@Adafruit]: https://www.adafruit.com/products/2127
[Spark@Spark]: https://store.spark.io
