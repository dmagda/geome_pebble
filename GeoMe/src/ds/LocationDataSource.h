#pragma once

#include "MessagingChannel.h"
#include "Contact.h"

typedef void (*LocationDataSourceDetermineLocationSuccessHandler)(int accuracy);
typedef void (*LocationDataSourceDetermineLocationFailureHandler)(MessageChannelResult reason);

typedef void (*LocationDataSourceSendLocationSuccessHandler)(int contactId);
typedef void (*LocationDataSourceSendLocationFailureHandler)(int contactId, MessageChannelResult reason);


typedef struct {
    LocationDataSourceDetermineLocationSuccessHandler determineLocationSuccess;
    LocationDataSourceDetermineLocationFailureHandler determineLocationFailure;
    LocationDataSourceSendLocationSuccessHandler sendLocationSuccess;
    LocationDataSourceSendLocationFailureHandler sendLocationFailure;
} LocationDataSourceHandlers;



void location_ds_set_handlers(LocationDataSourceHandlers handlers);

void location_ds_clear_handlers();

void location_ds_destroy();



MessageChannelResult location_ds_determine_current_location();

MessageChannelResult location_ds_send_location_to_contact(Contact *contact);



void location_ds_location_determined(Tuple *resultTuple);

void location_ds_location_determinitaion_failed(MessageChannelResult reason);

void location_ds_location_sent(Tuple *contactTuple);

void location_ds_location_sending_failed(Tuple *contactTuple, MessageChannelResult reason);

