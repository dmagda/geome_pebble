#pragma once

#define APP_NAME_TEXT "GeoMe"

#define LOADING_CONTACTS_TEXT "Loading contacts..."
#define UNABLE_TO_LOAD_CONTACTS_TEXT "Unable to load your contacts"

#define DETERMINE_LOCATION_TEXT "Finding your current location..."
#define SENDING_LOCATION_TEXT "Sending your location..."
#define SUCCESSFULLY_SENT_LOCATION "Your location is sent!"
#define UNABLE_TO_SEND_LOCATION "Unable to send your location"
#define LOW_ACCURACY_ALERT "Your current location accuracy is low, about %d meters.\nPress the middle button if you want to send it anyway."

#define ERROR_NOT_AUTHORIZED_TEXT "Authorize in GeoMe first.\nOpen Pebble app on your device, locate GeoMe in the installed apps list and press \"Settings\"."
#define ERROR_PHONE_NOT_CONNECTED_TEXT "Your phone is not responding.\nCheck that Pebble's phone app is running."
#define ERROR_PHONE_APP_NOT_RUNNING_TEXT ERROR_PHONE_NOT_CONNECTED_TEXT //IMPL_NOTE: this code is not returned right now
#define ERROR_HTTP_REQUEST_TIMEOUT_TEXT "Internet connection on your phone appears to be offline."
#define ERROR_LOCATION_NOT_DETERMINED_TEXT "Unable to find your location.\nCheck location services settings."
#define ERROR_GENERAL_TEXT "Oops, something has gone wrong. Please, try again."
