# BME280 Climate Sensor using HomeKit ADK

Based on the demo application for the [HomeKit ADK library](https://github.com/mongoose-os-libs/homekit-adk).

[HomeKit Accessory Protocol Specification ](https://developer.apple.com/homekit/specification/)

## Setup

 * Build and flash the firmware as usual.
 * Provision setup code via RPC:
```
 $ mos call HAP.Setup '{"code": "111-22-333"}'
Using port /dev/ttyUSB0
null
```
 * Set wifi credentials
```
 $ mos wifi MYSSID MYPASS
```
 * That's all! You should see "Acme Light Bulb 9000" appear in the list of accessories.

## Update with

    mos --port ws://<IP_ADDR>/rpc ota build/fw.zip
    url -v -F file=@build/fw.zip http://<IP_ADDR>/update


    mos --port ws://<IP_ADDR>/rpc call HAP.Setup '{"code": "111-22-333"}'
