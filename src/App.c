// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

// An example that implements the light bulb HomeKit profile. It can serve as a
// basic implementation for any platform. The accessory logic implementation is
// reduced to internal state updates and log output.
//
// This implementation is platform-independent.
//
// The code consists of multiple parts:
//
//   1. The definition of the accessory configuration and its internal state.
//
//   2. Helper functions to load and save the state of the accessory.
//
//   3. The definitions for the HomeKit attribute database.
//
//   4. The callbacks that implement the actual behavior of the accessory, in
//   this
//      case here they merely access the global accessory state variable and
//      write to the log to make the behavior easily observable.
//
//   5. The initialization of the accessory state.
//
//   6. Callbacks that notify the server in case their associated value has
//   changed.

#include "App.h"
#include "DB.h"

#include "mgos.h"
#include "mgos_hap.h"

#include "mgos_bme280.h"
#include "mgos_gpio.h"

void AccessoryNotification(const HAPAccessory *accessory,
                           const HAPService *service,
                           const HAPCharacteristic *characteristic,
                           void *ctx HAP_UNUSED);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Domain used in the key value store for application data.
 *
 * Purged: On factory reset.
 */
#define kAppKeyValueStoreDomain_Configuration \
  ((HAPPlatformKeyValueStoreDomain) 0x00)

/**
 * Key used in the key value store to store the configuration state.
 *
 * Purged: On factory reset.
 */
#define kAppKeyValueStoreKey_Configuration_State \
  ((HAPPlatformKeyValueStoreDomain) 0x00)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct mgos_bme280 *bme;

/**
 * Global accessory configuration.
 */
typedef struct {
  struct {
    float temperatureSensorCurrentTemperature;
  } state;
  HAPAccessoryServerRef *server;
  HAPPlatformKeyValueStoreRef keyValueStore;
} AccessoryConfiguration;

static AccessoryConfiguration accessoryConfiguration;

//----------------------------------------------------------------------------------------------------------------------

/**
 * Load the accessory state from persistent memory.
 */
static void LoadAccessoryState(void) {
  HAPPrecondition(accessoryConfiguration.keyValueStore);

  HAPError err;

  // Load persistent state if available
  bool found;
  size_t numBytes;

  err = HAPPlatformKeyValueStoreGet(
      accessoryConfiguration.keyValueStore,
      kAppKeyValueStoreDomain_Configuration,
      kAppKeyValueStoreKey_Configuration_State, &accessoryConfiguration.state,
      sizeof accessoryConfiguration.state, &numBytes, &found);

  if (err) {
    HAPAssert(err == kHAPError_Unknown);
    HAPFatalError();
  }
  if (!found || numBytes != sizeof accessoryConfiguration.state) {
    if (found) {
      HAPLogError(&kHAPLog_Default,
                  "Unexpected app state found in key-value store. Resetting to "
                  "default.");
    }
    HAPRawBufferZero(&accessoryConfiguration.state,
                     sizeof accessoryConfiguration.state);
  }
}

/**
 * Save the accessory state to persistent memory.
 */
// static void SaveAccessoryState(void) {
//   HAPPrecondition(accessoryConfiguration.keyValueStore);

//   HAPError err;
//   err = HAPPlatformKeyValueStoreSet(accessoryConfiguration.keyValueStore,
//                                     kAppKeyValueStoreDomain_Configuration,
//                                     kAppKeyValueStoreKey_Configuration_State,
//                                     &accessoryConfiguration.state,
//                                     sizeof accessoryConfiguration.state);
//   if (err) {
//     HAPAssert(err == kHAPError_Unknown);
//     HAPFatalError();
//   }
// }

//----------------------------------------------------------------------------------------------------------------------

/**
 * HomeKit accessory that provides the Sensor Service.
 *
 * Note: Not constant to enable BCT Manual Name Change.
 */
static HAPAccessory accessory = {
    .aid = 1,
    .category = kHAPAccessoryCategory_Sensors,
    .name = CS_STRINGIFY_MACRO(HAP_PRODUCT_NAME),
    .manufacturer = CS_STRINGIFY_MACRO(HAP_PRODUCT_VENDOR),
    .model = CS_STRINGIFY_MACRO(HAP_PRODUCT_MODEL),
    .serialNumber = NULL,     // Set from config.
    .firmwareVersion = NULL,  // Set from build_id.
    .hardwareVersion = CS_STRINGIFY_MACRO(HAP_PRODUCT_HW_REV),
    .services =
        (const HAPService *const[]){&mgos_hap_accessory_information_service,
                                    &mgos_hap_protocol_information_service,
                                    &mgos_hap_pairing_service,
                                    &temperatureSensorService,
                                    &humiditySensorService,
                                    NULL},
    .callbacks = {.identify = IdentifyAccessory}};

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError IdentifyAccessory(HAPAccessoryServerRef *server HAP_UNUSED,
                           const HAPAccessoryIdentifyRequest *request
                               HAP_UNUSED,
                           void *_Nullable context HAP_UNUSED) {
    HAPLogInfo(&kHAPLog_Default, "%s", __func__);
    return kHAPError_None;
}


HAP_RESULT_USE_CHECK
HAPError HandleTemperatureSensorRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPFloatCharacteristicReadRequest *request HAP_UNUSED,
    float *value,
    void *_Nullable context HAP_UNUSED
) {

    *value = mgos_bme280_read_temperature(bme);

    HAPLogInfo(&kHAPLog_Default, "%s: %f", __func__, *value);
    HAPAccessoryServerRaiseEvent(server, request->characteristic, request->service, request->accessory);

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleHumiditySensorRead(HAPAccessoryServerRef *server HAP_UNUSED,
                                  const HAPFloatCharacteristicReadRequest *request HAP_UNUSED,
                                  float *value,
                                  void *_Nullable context HAP_UNUSED
) {
    *value = mgos_bme280_read_humidity(bme);
    HAPLogInfo(&kHAPLog_Default, "%s: %f", __func__, *value);

    HAPAccessoryServerRaiseEvent(server, request->characteristic, request->service, request->accessory);

    return kHAPError_None;
}
//----------------------------------------------------------------------------------------------------------------------

void AccessoryNotification(const HAPAccessory *accessory,
                           const HAPService *service,
                           const HAPCharacteristic *characteristic,
                           void *ctx HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "Accessory Notification");

  HAPAccessoryServerRaiseEvent(accessoryConfiguration.server, characteristic,
                               service, accessory);
}

void AppCreate(HAPAccessoryServerRef *server,
               HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(server);
    HAPPrecondition(keyValueStore);

    HAPLogInfo(&kHAPLog_Default, "%s", __func__);

    HAPRawBufferZero(&accessoryConfiguration, sizeof accessoryConfiguration);
    accessoryConfiguration.server = server;
    accessoryConfiguration.keyValueStore = keyValueStore;
    LoadAccessoryState();
}

void AppRelease(void) {
}

void AppAccessoryServerStart(void) {
    HAPAccessoryServerStart(accessoryConfiguration.server, &accessory);
}

//----------------------------------------------------------------------------------------------------------------------

void AccessoryServerHandleUpdatedState(HAPAccessoryServerRef *server,
                                       void *_Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(!context);

    switch (HAPAccessoryServerGetState(server)) {
        case kHAPAccessoryServerState_Idle: {
        HAPLogInfo(&kHAPLog_Default, "Accessory Server State did update: Idle.");
        return;
        }
        case kHAPAccessoryServerState_Running: {
        HAPLogInfo(&kHAPLog_Default,
                    "Accessory Server State did update: Running.");
        return;
        }
        case kHAPAccessoryServerState_Stopping: {
        HAPLogInfo(&kHAPLog_Default,
                    "Accessory Server State did update: Stopping.");
        return;
        }
    }
    HAPFatalError();
}

HAPAccessory *AppGetAccessoryInfo() {
  return &accessory;
}

void AppInitialize(
    HAPAccessoryServerOptions *hapAccessoryServerOptions HAP_UNUSED,
    HAPPlatform *hapPlatform HAP_UNUSED,
    HAPAccessoryServerCallbacks *hapAccessoryServerCallbacks HAP_UNUSED) {

    accessory.firmwareVersion = mgos_sys_ro_vars_get_fw_version();
    accessory.serialNumber = mgos_sys_config_get_device_sn();
    temperatureSensorService.name = mgos_sys_config_get_temperaturesensor_name();
    humiditySensorService.name = mgos_sys_config_get_humiditysensor_name();

    bme = mgos_bme280_i2c_create(0x76);

    if (!bme) {
        HAPLogError(&kHAPLog_Default, "=== BME is NULL!");
        mgos_gpio_set_mode(13, MGOS_GPIO_MODE_OUTPUT);
        mgos_gpio_write(13, true);
    }

}

void AppDeinitialize() {
  /*no-op*/
  mgos_bme280_delete(bme);
}
