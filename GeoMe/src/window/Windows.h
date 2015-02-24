#pragma once

#include "Contact.h"


//Welcome window
void welcome_window_show();
void welcome_window_destroy();
void welcome_window_stop_transitioning();

//Contacts window
void contacts_window_show();
void contacts_window_destroy();

//Sending location window
void sending_location_window_show(Contact *contactToSend);
void sending_location_window_destroy();


//Authorization window
void authorization_window_show();
void authorization_window_hide();
void authorization_window_destroy();
