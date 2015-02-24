#pragma once

#include "MessagingChannel.h"
#include "Contact.h"

typedef void (*ContactsDataSourceSuccessHadler)(Contact **contacts, int count);
typedef void (*ContactsDataSourceFailureHandler)(MessageChannelResult reason);

typedef struct {
    ContactsDataSourceSuccessHadler success;
    ContactsDataSourceFailureHandler failure;
} ContactsDataSourceHandlers;


void contacts_ds_set_handlers(ContactsDataSourceHandlers handlers);

void contacts_ds_clear_handlers();

void contacts_ds_destroy();



void contacts_ds_get_contacts(Contact ***pContacts, int *count);

MessageChannelResult contacts_ds_fetch_contacts();



void contacts_ds_contacts_count_received(Tuple *contactsCountTuple);

void contacts_ds_contact_received(Tuple *contactIdTuple, Tuple *contactNameTuple);

void contacts_ds_failed_to_fetch_contacts(MessageChannelResult reason);


