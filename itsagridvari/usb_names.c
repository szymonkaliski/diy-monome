#include "usb_names.h"

#define PRODUCT_NAME      {'t','e','e','n','s','y','g','r','i','d'}
#define PRODUCT_NAME_LEN   10

struct usb_string_descriptor_struct usb_string_product_name = {
  2 + PRODUCT_NAME_LEN * 2,
  3,
  PRODUCT_NAME
};
