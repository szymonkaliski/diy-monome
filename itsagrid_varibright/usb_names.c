#include "usb_names.h"

//#define PRODUCT_NAME      {'t','e','e','n','s','y','g','r','i','d'}
#define PRODUCT_NAME      {'m','o','n','o','m','e','g','r','i','d'}
#define PRODUCT_NAME_LEN   10
#define MANUFACTURER_NAME  {'m','o','n','o','m','e'}
#define MANUFACTURER_NAME_LEN 6
#define SERIAL_NUM  {'m','4','6','7','6','0','0','0'}
#define SERIAL_NUM_LEN 8

struct usb_string_descriptor_struct usb_string_product_name = {
  2 + PRODUCT_NAME_LEN * 2,
  3,
  PRODUCT_NAME
};

struct usb_string_descriptor_struct usb_string_manufacturer_name = {
  2 + MANUFACTURER_NAME_LEN * 2,
  3,
  MANUFACTURER_NAME
};

struct usb_string_descriptor_struct usb_string_serial_number = {
  2 + SERIAL_NUM_LEN * 2,
  3,
  SERIAL_NUM
};
