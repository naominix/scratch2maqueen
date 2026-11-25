#include "pxt.h"

#if MICROBIT_CODAL && CONFIG_ENABLED(DEVICE_BLE)
#include "MicroBitBLEManager.h"
#include "MicroBitBLEService.h"

using namespace codal;

namespace {

// The primary service uses a 16-bit UUID and the two characteristics share a vendor base.
constexpr uint16_t serviceUuid = 0xf005;
constexpr uint8_t characteristicBase[] = {
    0x52, 0x61, 0x00, 0x00, 0xfa, 0x7e, 0x42, 0xab,
    0x85, 0x0b, 0x7c, 0x80, 0x22, 0x00, 0x97, 0xcc
};

class ScratchMaqueenService : public MicroBitBLEService {
    enum { Notify, Write, CharacteristicCount };
    static constexpr int packetSize = 20;
    MicroBitBLEChar characteristics[CharacteristicCount] = {};
    uint8_t notifyValue[packetSize] = {};
    uint8_t writeValue[packetSize] = {};
    int16_t slots[4] = {};
    uint8_t pendingDisplay[packetSize] = {};
    int pendingLength = 0;

    void resetCommands() {
        memset(slots, 0, sizeof(slots));
        pendingLength = 0;
    }

public:
    ScratchMaqueenService() {
        bs_uuid_type = BLE_UUID_TYPE_BLE;
        CreateService(serviceUuid);
        RegisterBaseUUID(characteristicBase);
        CreateCharacteristic(Notify, 0xda01, notifyValue,
                             packetSize, packetSize, microbit_propREAD | microbit_propNOTIFY);
        CreateCharacteristic(Write, 0xda02, writeValue, 0, packetSize,
                             microbit_propWRITE | microbit_propWRITE_WITHOUT);
    }

    int characteristicCount() override { return CharacteristicCount; }
    MicroBitBLEChar *characteristicPtr(int index) override { return &characteristics[index]; }

    void onConnect(const microbit_ble_evt_t *) override { resetCommands(); }
    void onDisconnect(const microbit_ble_evt_t *) override { resetCommands(); }

    void onDataWritten(const microbit_ble_evt_write_t *event) override {
        if (event->handle != valueHandle(Write) || event->offset != 0 ||
            event->len == 0 || event->len > packetSize) {
            return;
        }
        const uint8_t *data = event->data;
        if (data[0] == 0xa0 && event->len == 4) {
            const unsigned slot = data[1];
            const int word = data[2] | (data[3] << 8);
            const int value = word >= 0x8000 ? word - 0x10000 : word;
            if (slot < 2) {
                slots[slot] = value;
            } else if (slot == 3) {
                slots[3] = value & 0x0c;
            }
        } else if (data[0] == 0x81 || (data[0] == 0x82 && event->len == 6)) {
            memcpy(pendingDisplay, data, event->len);
            pendingLength = event->len;
        }
    }

    int command(int slot) {
        return slot >= 0 && slot < 4 && getConnected() ? slots[slot] : 0;
    }

    void publish(pxt::Buffer frame) {
        if (!getConnected() || frame->length != packetSize) {
            return;
        }
        notifyChrValue(Notify, frame->data, packetSize);

        // Run display work in the MakeCode fiber instead of the BLE callback.
        const int len = pendingLength;
        if (len == 0) {
            return;
        }
        uint8_t command[packetSize];
        memcpy(command, pendingDisplay, len);
        pendingLength = 0;
        auto &display = pxt::uBit.display;
        display.stopAnimation();
        if (command[0] == 0x81) {
            char text[packetSize];
            memcpy(text, command + 1, len - 1);
            text[len - 1] = '\0';
            display.scrollAsync(ManagedString(text), 120);
        } else {
            for (int y = 0; y < 5; ++y) {
                for (int x = 0; x < 5; ++x) {
                    display.image.setPixelValue(x, y, (command[y + 1] & (1 << x)) ? 255 : 0);
                }
            }
        }
    }
};

ScratchMaqueenService *service = nullptr;

uint32_t advertiseLegacyUuid() {
    // CODAL has already allocated its first advertising set (handle 0).
    // Reconfigure that set so CODAL can restart it after a disconnect.
    auto *manager = MicroBitBLEManager::getInstance();
    manager->stopAdvertising();

    static uint8_t payload[BLE_GAP_ADV_SET_DATA_SIZE_MAX] = {
        2, BLE_GAP_AD_TYPE_FLAGS, BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,
        3, BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE, 0x05, 0xf0
    };
    uint16_t nameLength = sizeof(payload) - 9;
    const uint32_t nameStatus = sd_ble_gap_device_name_get(payload + 9, &nameLength);
    uint16_t payloadLength = 7;
    if (nameStatus == NRF_SUCCESS && nameLength <= sizeof(payload) - 9) {
        payload[7] = nameLength + 1;
        payload[8] = BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME;
        payloadLength = 9 + nameLength;
    }

    ble_gap_adv_data_t data = {};
    data.adv_data.p_data = payload;
    data.adv_data.len = payloadLength;

    ble_gap_adv_params_t params = {};
    params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    params.interval = 320; // 200 ms in SoftDevice units.
    params.filter_policy = BLE_GAP_ADV_FP_ANY;
    params.primary_phy = BLE_GAP_PHY_1MBPS;

    uint8_t handle = 0;
    const uint32_t status = sd_ble_gap_adv_set_configure(&handle, &data, &params);
    if (status != NRF_SUCCESS) {
        return status;
    }
    manager->setAdvertiseOnDisconnect(true);
    // CODAL initializes the micro:bit SoftDevice connection configuration with tag 1.
    return sd_ble_gap_adv_start(handle, 1);
}

} // namespace
#endif

#if !MICROBIT_CODAL
#include "MicroBit.h"

// DAL (micro:bit V1) implementation of the same service. The wire protocol
// is identical to the CODAL service above; the BLE plumbing follows the
// pattern proven by yokobond/pxt-scratch-more v0.3.2 (MIT) on V1 hardware.
namespace {

const uint16_t dalServiceUuid = 0xf005;
const uint8_t dalNotifyUuid[16] = {
    0x52, 0x61, 0xda, 0x01, 0xfa, 0x7e, 0x42, 0xab,
    0x85, 0x0b, 0x7c, 0x80, 0x22, 0x00, 0x97, 0xcc
};
const uint8_t dalWriteUuid[16] = {
    0x52, 0x61, 0xda, 0x02, 0xfa, 0x7e, 0x42, 0xab,
    0x85, 0x0b, 0x7c, 0x80, 0x22, 0x00, 0x97, 0xcc
};

class ScratchMaqueenServiceDal {
    static constexpr int packetSize = 20;
    uint8_t notifyValue[packetSize] = {};
    uint8_t writeValue[packetSize] = {};
    int16_t slots[4] = {};
    uint8_t pendingDisplay[packetSize] = {};
    int pendingLength = 0;
    GattAttribute::Handle_t notifyHandle = 0;
    GattAttribute::Handle_t writeHandle = 0;

    void resetCommands() {
        memset(slots, 0, sizeof(slots));
        pendingLength = 0;
    }

    bool connected() {
        return uBit.ble && uBit.ble->gap().getState().connected;
    }

public:
    ScratchMaqueenServiceDal() {
        GattCharacteristic notifyChr(
            dalNotifyUuid, notifyValue, 0, packetSize,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        GattCharacteristic writeChr(
            dalWriteUuid, writeValue, 0, packetSize,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
        notifyChr.requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);
        writeChr.requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

        GattCharacteristic *characteristics[] = {&notifyChr, &writeChr};
        GattService service(dalServiceUuid, characteristics,
                            sizeof(characteristics) / sizeof(GattCharacteristic *));
        uBit.ble->addService(service);
        notifyHandle = notifyChr.getValueHandle();
        writeHandle = writeChr.getValueHandle();
        uBit.ble->gattServer().write(notifyHandle, notifyValue, packetSize);

        const uint16_t uuid16List[] = {dalServiceUuid};
        uBit.ble->gap().accumulateAdvertisingPayload(
            GapAdvertisingData::INCOMPLETE_LIST_16BIT_SERVICE_IDS,
            (uint8_t *)uuid16List, sizeof(uuid16List));

        uBit.ble->onDataWritten(this, &ScratchMaqueenServiceDal::onDataWritten);
        uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_CONNECTED, this,
                               &ScratchMaqueenServiceDal::onConnected,
                               MESSAGE_BUS_LISTENER_IMMEDIATE);
        uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_DISCONNECTED, this,
                               &ScratchMaqueenServiceDal::onDisconnected,
                               MESSAGE_BUS_LISTENER_IMMEDIATE);
    }

    void onConnected(MicroBitEvent) { resetCommands(); }
    void onDisconnected(MicroBitEvent) { resetCommands(); }

    void onDataWritten(const GattWriteCallbackParams *params) {
        if (params->handle != writeHandle || params->len == 0 ||
            params->len > packetSize) {
            return;
        }
        const uint8_t *data = params->data;
        if (data[0] == 0xa0 && params->len == 4) {
            const unsigned slot = data[1];
            const int word = data[2] | (data[3] << 8);
            const int value = word >= 0x8000 ? word - 0x10000 : word;
            if (slot < 2) {
                slots[slot] = value;
            } else if (slot == 3) {
                slots[3] = value & 0x0c;
            }
        } else if (data[0] == 0x81 || (data[0] == 0x82 && params->len == 6)) {
            memcpy(pendingDisplay, data, params->len);
            pendingLength = params->len;
        }
    }

    int command(int slot) {
        return slot >= 0 && slot < 4 && connected() ? slots[slot] : 0;
    }

    void publish(pxt::Buffer frame) {
        if (!connected() || frame->length != packetSize) {
            return;
        }
        uBit.ble->gattServer().notify(notifyHandle, frame->data, packetSize);

        // Run display work in the MakeCode fiber instead of the BLE callback.
        const int len = pendingLength;
        if (len == 0) {
            return;
        }
        uint8_t command[packetSize];
        memcpy(command, pendingDisplay, len);
        pendingLength = 0;
        auto &display = uBit.display;
        display.stopAnimation();
        if (command[0] == 0x81) {
            char text[packetSize];
            memcpy(text, command + 1, len - 1);
            text[len - 1] = '\0';
            display.scrollAsync(ManagedString(text), 120);
        } else {
            for (int y = 0; y < 5; ++y) {
                for (int x = 0; x < 5; ++x) {
                    display.image.setPixelValue(x, y, (command[y + 1] & (1 << x)) ? 255 : 0);
                }
            }
        }
    }
};

ScratchMaqueenServiceDal *dalService = nullptr;

} // namespace
#endif

namespace LegacyBle {

//%
int start() {
#if MICROBIT_CODAL && CONFIG_ENABLED(DEVICE_BLE)
    if (!service) {
        service = new ScratchMaqueenService();
    }
    return advertiseLegacyUuid();
#elif !MICROBIT_CODAL
    if (!dalService) {
        dalService = new ScratchMaqueenServiceDal();
    }
    return 0;
#else
    return -1;
#endif
}

//%
int command(int slot) {
#if MICROBIT_CODAL && CONFIG_ENABLED(DEVICE_BLE)
    return service ? service->command(slot) : 0;
#elif !MICROBIT_CODAL
    return dalService ? dalService->command(slot) : 0;
#else
    return 0;
#endif
}

//%
void publish(pxt::Buffer frame) {
#if MICROBIT_CODAL && CONFIG_ENABLED(DEVICE_BLE)
    if (service) {
        service->publish(frame);
    }
#elif !MICROBIT_CODAL
    if (dalService) {
        dalService->publish(frame);
    }
#endif
}

} // namespace LegacyBle
