/**
 * @file msiklm.c
 *
 * @brief source file that contains controller functions to configure the SteelSeries keyboard in MSI gaming notebooks
 */

#include "msiklm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int hex(unsigned char c) {

        if (c >= '0' && c <='9')
                return c - '0';
        if (c >= 'A' && c <= 'F')
                return 10 + c - 'A';
        if (c >= 'a' && c <= 'f')
                return 10 + c - 'a';
        return 0;
}

/**
 * @brief parses a string into a color value
 * @param color_str the color value as a string (red, green, blue, etc.) or in [r;g;b] notation where r;g;b are the respective channel values
 * @param result the parsed color
 * @returns 0 if parsing succeeded, -1 on error
 */
int parse_color(const char* color_str, color_t* result)
{
    struct _s { char *name; color_t color; } color_map[9] = { { "none", none },
                                                          { "red", red },
                                                          { "orange", orange },
                                                          { "yellow", yellow },
                                                          { "green", green },
                                                          { "cyan", cyan },
                                                          { "blue", blue },
                                                          { "purple", purple },
                                                          { "white", white }
                                                        };  
    if (color_str != NULL)
    {
        // if it's an hexa code, parse it
        if (color_str[0] == '0' && color_str[1] == 'x')
        {
            const char *hex_value = color_str + 2;

            if (strlen(hex_value) == 6)
            {
                result->red = hex(hex_value[0]) * 16 + hex(hex_value[1]);
                result->green = hex(hex_value[2]) * 16 + hex(hex_value[3]);
                result->blue = hex(hex_value[4]) * 16 + hex(hex_value[5]);
		return 0;
            }
        } 
        else
        {
            int i;
            for( i = 0; i < 9; i++)
            {
                if (strcmp(color_str, color_map[i].name) == 0)
                {
                    *result = color_map[i].color;
                    return 0;
                }
            }
        }
     } 
    
    return -1;
}

/**
 * @brief parses a string into a brightness value
 * @param brightness_str the brightness value as a string
 * @returns the parsed brightness value or -1 if the string is not a valid brightness
 */
enum brightness parse_brightness(const char* brightness_str)
{
    enum brightness ret = -1;
    if (brightness_str != NULL)
    {
        switch (brightness_str[0])
        {
            case 'h':
                if (strcmp(brightness_str, "high") == 0)
                    ret = high;
                break;

            case 'm':
                if (strcmp(brightness_str, "medium") == 0)
                    ret = medium;
                break;

            case 'l':
                if (strcmp(brightness_str, "low") == 0)
                    ret = low;
                break;

            case 'o':
                if (strcmp(brightness_str, "off") == 0)
                    ret = off;
                break;
        }
    }
    return ret;
}

/**
 * @brief parses a string into a mode value
 * @param mode_str the mode value as a string
 * @returns the parsed mode value or -1 if the string is not a valid mode
 */
enum mode parse_mode(const char* mode_str)
{
    enum mode ret = -1;
    if (mode_str != NULL)
    {
        switch (mode_str[0])
        {
            case 'n':
                if (strcmp(mode_str, "normal") == 0)
                    ret = normal;
                break;

            case 'g':
                if (strcmp(mode_str, "gaming") == 0)
                    ret = gaming;
                break;

            case 'b':
                if (strcmp(mode_str, "breathe") == 0)
                    ret = breathe;
                break;

            case 'd':
                if (strcmp(mode_str, "demo") == 0)
                    ret = demo;
                break;

            case 'w':
                if (strcmp(mode_str, "wave") == 0)
                    ret = wave;
                break;
        }
    }
    return ret;
}

/**
 * @brief tries to open the MSI gaming notebook's SteelSeries keyboard and if it succeeds, it will be closed
 * @returns true, if the keyboard could be opened, false otherwise
 */
bool keyboard_found()
{
    hid_device* dev = open_keyboard();
    bool ret = dev != NULL;
    if (ret)
        hid_close(dev);
    return ret;
}

/**
 * @brief tries to open the MSI gaming notebook's SteelSeries keyboard
 * @returns a corresponding hid_device, null if the keyboard was not detected
 */
hid_device* open_keyboard()
{
    hid_device* dev = NULL;
    if (hid_init() == 0)
        dev = hid_open(0x1770, 0xff00, 0);
    return dev;
}

/**
 * @brief sets the selected color for a specified region (the colors will only be set as soon as set_mode() is called in advance)
 * @param dev the hid device
 * @param color the color value
 * @param region the region where the color should be set_color
 * @param brightness the selected brightness
 * @returns the acutal number of bytes written, -1 on error
 */
int set_color(hid_device* dev, color_t color, enum region region, enum brightness brightness)
{
    int ret;
    if (region == left || region == middle || region == right || region == logo || region == front_left || region == front_right || region == mouse)
    {
        if (brightness == medium || brightness == low || brightness == off)
        {
            double factor = brightness / (-3.0) + 1.0;
            color.red   = (byte)(factor * color.red);
            color.green = (byte)(factor * color.green);
            color.blue  = (byte)(factor * color.blue);
            //printf("Color = [%i, %i, %i]\n", color.red, color.green, color.blue);
        }

        byte buffer[8];
        buffer[0] = 1;
        buffer[1] = 2;
        buffer[2] = 64; // rgb
        buffer[3] = (byte)region;
        buffer[4] = color.red;
        buffer[5] = color.green;
        buffer[6] = color.blue;
        buffer[7] = 236; // EOR (end of request)

        ret = hid_send_feature_report(dev, buffer, 8);

        // alternative color setting, slightly simplier but allows less configuration options:
        //enum color { none=0, red=1, orange=2, yellow=3, green=4, sky=5, blue=6, purple=7, white=8 }
        //byte buffer[8];
        //buffer[0] = 1;
        //buffer[1] = 2;
        //buffer[2] = 66; // set
        //buffer[3] = (byte)region;
        //buffer[4] = (byte)color;
        //buffer[5] = (byte)brightness;
        //buffer[6] = 0;
        //buffer[7] = 236; // EOR (end of request)
    }
    else
    {
        ret = -1;
    }
    return ret;
}

/**
 * @brief sets the selected mode
 * @param dev the hid device
 * @param mode the selected mode
 * @returns the acutal number of bytes written, -1 on error
 */
int set_mode(hid_device* dev, enum mode mode)
{
    // check for a valid value, otherwise use normal mode
    if (mode != normal  && mode != gaming && mode != breathe && mode != demo && mode != wave)
        return -1;

    byte buffer[8];
    buffer[0] = 1;
    buffer[1] = 2;
    buffer[2] = 65; // commit
    buffer[3] = (byte)mode; // set hardware mode
    buffer[4] = 0;
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 236; // EOR (end of request)

    return hid_send_feature_report(dev, buffer, 8);
}

/**
 * @brief iterates through all found HID devices
 */
void enumerate_hid()
{
    struct hid_device_info* enumerate = hid_enumerate(0,0);

    if (enumerate != NULL)
    {
        struct hid_device_info* dev = enumerate;
        while (dev != NULL)
        {
            printf("Device: %S\n", dev->product_string);
            printf("    Device Vendor ID:        %i\n", dev->vendor_id);
            printf("    Device Product ID:       %i\n", dev->product_id);
            printf("    Device Serial Number:    %S\n", dev->serial_number);
            printf("    Device Manufacturer:     %S\n", dev->manufacturer_string);
            printf("    Device Path:             %s\n", dev->path);
            printf("    Device Interface Number: %i\n", dev->interface_number);
            printf("    Device Release Number:   %d\n", dev->release_number);
            printf("\n");
            dev = dev->next;
        }
        hid_free_enumeration(enumerate);
    }
    else
    {
        printf("No HID device found!\n");
    }
}

