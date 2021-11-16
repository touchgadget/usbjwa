/************************************************************************
MIT License

Copyright (c) 2021 touchgadgetdev@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*************************************************************************/
#pragma once
#include "USB.h"
#include "USBHID.h"

#if CONFIG_TINYUSB_HID_ENABLED

#define MAX_AXIS    32767

#define ATTRIBUTE_PACKED  __attribute__((packed, aligned(1)))

// This overlays the HID report defined by desc_hid_report[].
typedef struct ATTRIBUTE_PACKED {
    uint32_t  buttons;
    int16_t   xAxis;
    int16_t	yAxis;
} HID_JoystickReport_Data_t;

// HID report descriptor 2 axes (16 bits per axis), 32 buttons
// Single Report (no report ID) descriptor
uint8_t const desc_hid_report[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x04,        // Usage (Joystick)
    0xA1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x20,        //   Report Count (32)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x20,        //   Usage Maximum (0x20)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x16, 0x01, 0x80,  //   Logical Minimum (-32767)
    0x36, 0x01, 0x80,  //   Physical Minimum (-32767)
    0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
    0x46, 0xFF, 0x7F,  //   Physical Maximum (32767)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
};


class TUJoystick: public USBHIDDevice {
    public:
        inline TUJoystick(void);

        inline void begin(void);
        inline void end(void);
        inline void loop(void);
        inline void write(void) { this->usb_hid.SendReport(0, &this->_report, (uint8_t)sizeof(this->_report)); };
        inline void write(void *report);
        inline void press(int b);
        inline void release(int b);
        inline void releaseAll(void) { this->_report.buttons = 0; };

        inline void buttons(uint32_t b) { this->_report.buttons = b; };
        inline void xAxis(int a) { this->_report.xAxis = a; };
        inline void yAxis(int a) { this->_report.yAxis = a; };

        inline bool ready(void) { return this->usb_hid.ready(); };
        uint16_t _onGetDescriptor(uint8_t* buffer);

    protected:
        HID_JoystickReport_Data_t _report;
        uint32_t _start_millis;
        USBHID usb_hid;
};

TUJoystick::TUJoystick(): usb_hid()
{
    usb_hid.addDevice(this, sizeof(desc_hid_report));
}

uint16_t TUJoystick::_onGetDescriptor(uint8_t *dst)
{
    memcpy(dst, desc_hid_report, sizeof(desc_hid_report));
    return sizeof(desc_hid_report);
}

void TUJoystick::begin(void)
{
    USB.usbClass(0);
    USB.usbSubClass(0);
    USB.usbProtocol(0);
    this->usb_hid.begin();
    
    // release all buttons, center all sticks, etc.
    end();
    this->_start_millis = millis();
}

// Avoid USB writes more than once per millisecond
void TUJoystick::loop(void)
{
    if (this->_start_millis != millis()) {
        this->write();
        this->_start_millis = millis();
    }
}

void TUJoystick::end(void)
{
    this->_report.buttons = 0;
    this->_report.xAxis = this->_report.yAxis = 0;
    this->write();
}

void TUJoystick::write(void *report)
{
    memcpy(&this->_report, report, sizeof(this->_report));
    this->write();
}

void TUJoystick::press(int b)
{
    b &= 0x1F; // Limit value between 0..31
    this->_report.buttons |= (uint32_t)1 << b;
}

void TUJoystick::release(int b)
{
    b &= 0x1F; // Limit value between 0..31
    this->_report.buttons &= ~((uint32_t)1 << b);
}
#endif /* CONFIG_TINYUSB_HID_ENABLED */
