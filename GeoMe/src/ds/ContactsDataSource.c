#include "ContactsDataSource.h"

#define CONTACTS_FETCH_VALUE 1

//contacts related data and structures
static Contact **contacts = NULL;

static int contactsCount = 0;
static int contactsReceived = 0;
static bool isLoadingContacts = false;

static ContactsDataSourceSuccessHadler successHandler = NULL;
static ContactsDataSourceFailureHandler failureHandler = NULL;

static void release_current_contacts();



//---- public functions ----

void contacts_ds_set_handlers(ContactsDataSourceHandlers handlers)
{
  successHandler = handlers.success;
  failureHandler = handlers.failure;
}

void contacts_ds_clear_handlers()
{
  successHandler = NULL;
  failureHandler = NULL;
}

void contacts_ds_destroy()
{
  isLoadingContacts = false;
  contacts_ds_clear_handlers();
  release_current_contacts();
}



void contacts_ds_get_contacts(Contact ***pContacts, int *count)
{
  if (pContacts != NULL) {
    *pContacts = isLoadingContacts ? NULL : contacts;
  }

  if (count != NULL) {
    *count = isLoadingContacts ? 0 : contactsReceived;
  }
}

MessageChannelResult contacts_ds_fetch_contacts()
{
  if (isLoadingContacts) {
    return MSG_CHN_OK;
  }

  DictionaryIterator *iterator;

  AppMessageResult boxResult = app_message_outbox_begin(&iterator);

  if (boxResult != APP_MSG_OK) {
    return message_channel_convert_result_code(boxResult);
  }

  Tuplet tuple = TupletInteger(MSG_CHN_FETCH_CONTACTS_KEY, CONTACTS_FETCH_VALUE);
  DictionaryResult dictResult = dict_write_tuplet(iterator, &tuple);
  if (dictResult != DICT_OK) {
    //no way to release the iterator in API
    return MSG_CHN_GENERAL_ERROR;
  }

  dict_write_end(iterator);

  boxResult = app_message_outbox_send();

  if (boxResult == APP_MSG_OK) {
    isLoadingContacts = true;
  }

  return message_channel_convert_result_code(boxResult);
}


void contacts_ds_contacts_count_received(Tuple *contactsCountTuple)
{
  release_current_contacts();

  contactsCount = contactsCountTuple->value->int32;

  int contactsSize = sizeof(Contact*) * contactsCount;

  contacts = (Contact**)malloc(contactsSize);
  if (contacts == NULL) {
    contacts_ds_failed_to_fetch_contacts(MSG_CHN_GENERAL_ERROR);
    return;
  }

  memset(contacts, 0, contactsSize);
}

void contacts_ds_contact_received(Tuple *contactIdTuple, Tuple *contactNameTuple)
{
  if (contacts == NULL) {
    //OOM was generated at contacts_transmission_started stage
    return;
  }

  Contact *contact = malloc(sizeof(Contact));
  if (contact == NULL) {
    release_current_contacts();
    contacts_ds_failed_to_fetch_contacts(MSG_CHN_GENERAL_ERROR);
    return;
  }
  memset(contact, 0, sizeof(Contact));

  int nameLen = strlen(contactNameTuple->value->cstring);

  contact->name = malloc(nameLen + 1);
  if (contact->name == NULL) {
    contactsReceived++;
    release_current_contacts();
    contacts_ds_failed_to_fetch_contacts(MSG_CHN_GENERAL_ERROR);
    return;
  }

  strncpy(contact->name, contactNameTuple->value->cstring, nameLen);
  contact->name[nameLen] = '\0';

  contact->id = contactIdTuple->value->int32;

  contacts[contactsReceived++] = contact;

  if (contactsReceived == contactsCount) {
    isLoadingContacts = false;

    if (successHandler) {
      successHandler(contacts, contactsCount);
    }
  }
}


void contacts_ds_failed_to_fetch_contacts(MessageChannelResult reason)
{
  isLoadingContacts = false;

  if (failureHandler) {
    failureHandler(reason);
  }
}

//---- private functions ----
static void release_current_contacts()
{
  if (contacts == NULL) {
    return;
  }

  for (int i = 0; i < contactsReceived; i++) {
    Contact *contact = contacts[i];
    if (contact == NULL) {
      continue;
    }

    if (contact->name) {
      free(contact->name);
    }
    free(contact);
  }

  free(contacts);
  contacts = NULL;

  contactsCount = 0;
  contactsReceived = 0;
}

