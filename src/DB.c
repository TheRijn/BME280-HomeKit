// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

// This file contains the accessory attribute database that defines the
// accessory information service, HAP Protocol Information Service, the Pairing
// service and finally the service signature exposed by the light bulb.

#include "DB.h"
#include "App.h"

#include "mgos.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * IID constants.
 */
#define kIID_TemeratureSensor ((uint64_t) 0x0040)
#define kIID_TemperatureSensorServiceSignature ((uint64_t) 0x0041)
#define kIID_TemeratureSensorName ((uint64_t) 0x0042)
#define kIID_TemeratureSensorCurrentTemperature ((uint64_t) 0x0043)

#define kIID_HumiditySensor ((uint64_t) 0x0050)
#define kIID_HumiditySensorServiceSignature ((uint64_t) 0x0051)
#define kIID_HumiditySensorName ((uint64_t) 0x0052)
#define kIID_HumiditySensorCurrentRelativeHumidity ((uint64_t) 0x0053)

// HAP_STATIC_ASSERT(kAttributeCount == 9 + 3 + 5 + 3, AttributeCount_mismatch);

/**
 * The 'Service Signature' characteristic of the Temperature Sensor service.
 */
static const HAPDataCharacteristic temperatureSensorServiceSignatureCharacteristic = {
    .format = kHAPCharacteristicFormat_Data,
    .iid = kIID_TemperatureSensorServiceSignature,
    .characteristicType = &kHAPCharacteristicType_ServiceSignature,
    .debugDescription = kHAPCharacteristicDebugDescription_ServiceSignature,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = true},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 2097152},
    .callbacks = {.handleRead = HAPHandleServiceSignatureRead,
                  .handleWrite = NULL}};

/**
 * The 'Service Signature' characteristic of the Humidity Sensor service.
 */
static const HAPDataCharacteristic humiditySensorServiceSignatureCharacteristic = {
    .format = kHAPCharacteristicFormat_Data,
    .iid = kIID_HumiditySensorServiceSignature,
    .characteristicType = &kHAPCharacteristicType_ServiceSignature,
    .debugDescription = kHAPCharacteristicDebugDescription_ServiceSignature,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = true},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 2097152},
    .callbacks = {.handleRead = HAPHandleServiceSignatureRead,
                  .handleWrite = NULL}};

/**
 * The 'Name' characteristic of the Temperature Sensor service.
 */
static const HAPStringCharacteristic temperatureSensorNameCharacteristic = {
    .format = kHAPCharacteristicFormat_String,
    .iid = kIID_TemeratureSensorName,
    .characteristicType = &kHAPCharacteristicType_Name,
    .debugDescription = kHAPCharacteristicDebugDescription_Name,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 64},
    .callbacks = {.handleRead = HAPHandleNameRead, .handleWrite = NULL}};

/**
 * The 'Name' characteristic of the Humidity Sensor service.
 */
static const HAPStringCharacteristic humiditySensorNameCharacteristic = {
    .format = kHAPCharacteristicFormat_String,
    .iid = kIID_HumiditySensorName,
    .characteristicType = &kHAPCharacteristicType_Name,
    .debugDescription = kHAPCharacteristicDebugDescription_Name,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 64},
    .callbacks = {.handleRead = HAPHandleNameRead, .handleWrite = NULL}};

/**
 * The 'Current Temperature' characteristic of the Temperature Sensor service.
 */
const HAPFloatCharacteristic temperatureSensorCurrentTemperatureCharacteristic = {
    .format = kHAPCharacteristicFormat_Float,
    .iid = kIID_TemeratureSensorCurrentTemperature,
    .characteristicType = &kHAPCharacteristicType_CurrentTemperature,
    .debugDescription = kHAPCharacteristicDebugDescription_CurrentTemperature,
    .manufacturerDescription = NULL,
    .properties = {
        .readable = true,
        .writable = false,
        .hidden = false,
        .requiresTimedWrite = false,
        .supportsAuthorizationData = false,
        .supportsEventNotification = true,
        .ip = {
            .controlPoint = false,
            .supportsWriteResponse = false
        },
        .ble = {
            .supportsBroadcastNotification = false,
            .supportsDisconnectedNotification = false,
            .readableWithoutSecurity = false,
            .writableWithoutSecurity = false
        },
    },
    .units = kHAPCharacteristicUnits_Celsius,
    .constraints = {
        .minimumValue = 0,
        .maximumValue = 100,
        .stepValue = 0.1
    },
    .callbacks = {
        .handleRead = HandleTemperatureSensorRead,
        .handleWrite = NULL
        }
};


/**
 * The 'Current Relative Humidity' characteristic of the Humidity Sensor service.
 */
const HAPFloatCharacteristic humiditySensorCurrentRelativeHumidityCharacteristic = {
    .format = kHAPCharacteristicFormat_Float,
    .iid = kIID_HumiditySensorCurrentRelativeHumidity,
    .characteristicType = &kHAPCharacteristicType_CurrentRelativeHumidity,
    .debugDescription = kHAPCharacteristicDebugDescription_CurrentRelativeHumidity,
    .manufacturerDescription = NULL,
    .properties = {
        .readable = true,
        .writable = false,
        .hidden = false,
        .requiresTimedWrite = false,
        .supportsAuthorizationData = false,
        .supportsEventNotification = true,
        .ip = {
            .controlPoint = false,
            .supportsWriteResponse = false
        },
        .ble = {
            .supportsBroadcastNotification = false,
            .supportsDisconnectedNotification = false,
            .readableWithoutSecurity = false,
            .writableWithoutSecurity = false
        },
    },
    .units = kHAPCharacteristicUnits_Percentage,
    .constraints = {
        .minimumValue = 0,
        .maximumValue = 100,
        .stepValue = 1
    },
    .callbacks = {
        .handleRead = HandleHumiditySensorRead,
        .handleWrite = NULL
        }
};

/**
 * The Humidity Sensor service that contains the 'Current Relative Humidity' characteristic.
 */
HAPService humiditySensorService = {
    .iid = kIID_HumiditySensor,
    .serviceType = &kHAPServiceType_HumiditySensor,
    .name = NULL,
    .properties = {
        .primaryService = false,
        .hidden = false,
        .ble = {.supportsConfiguration = false}
    },
    .linkedServices = NULL,
    .characteristics = (const HAPCharacteristic *const[]){
        &humiditySensorServiceSignatureCharacteristic,
        &humiditySensorNameCharacteristic,
        &humiditySensorCurrentRelativeHumidityCharacteristic,
        NULL
    }
};

/**
 * The Temperature Sensor service that contains the 'Current Temperature' characteristic.
 */
HAPService temperatureSensorService = {
    .iid = kIID_TemeratureSensor,
    .serviceType = &kHAPServiceType_TemperatureSensor,
    .name = NULL,
    .properties = {
        .primaryService = true,
        .hidden = false,
        .ble = {.supportsConfiguration = false}
    },
    .linkedServices = NULL,
    .characteristics = (const HAPCharacteristic *const[]){
        &temperatureSensorServiceSignatureCharacteristic,
        &temperatureSensorNameCharacteristic,
        &temperatureSensorCurrentTemperatureCharacteristic,
        NULL
    }
};
