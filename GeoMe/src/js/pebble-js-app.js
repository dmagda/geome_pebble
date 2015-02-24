//--------- Global Data --------
var accessToken = null;

var requestTimeout = 20000; //20 seconds

var messageSendingAttemptsCount = 3;
var messageSendingAttemptsLeft = messageSendingAttemptsCount;
var messageSendingAttemptsInterval = 3000; //3 seconds

var maxContactNameLength = 16;

var locationOptions = {"timeout": 10000, "maximumAge": 100, "enableHighAccuracy": true};
var locationWatcher = null;
var lastCoordinate = null;
var locationError = null;
var contactIdToSendCoordinate = null;
var isLocationBeingSent = false;
var isLocationDeterminationDemanded = false;

var errorTypes = {
  "noError": 0,
  "notAuthorized" : 1,
  "notConnected" : 2,
  "phoneAppNotRunning" : 3,
  "httpTimeout" : 4,
  "locationIsNotDetermined": 5,
  "generalError" : 6
};


//-------- Listeners -----------

Pebble.addEventListener("ready",
    function(e) {
        console.log("GeoMe app started!");

        loadAccessToken();

        locationWatcher = window.navigator.geolocation.watchPosition(
          locationSuccess, locationError, locationOptions);
    }
);

Pebble.addEventListener("showConfiguration", function() {
  if (accessToken == null) {
    console.log("showing login window");
    login();

  } else {
    console.log("showing logout window");
    logout();
  }
});


Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration window closed");

  var response = JSON.parse(decodeURIComponent(e.response));

  console.log("webview response = " + JSON.stringify(response));

  if (response.action === "login") {
    if (response.accessToken !== "undefined") {
      onUserLogin(response.accessToken);
    }

  } else if (response.action === "logout") {
    onUserLogout();
  }
});

Pebble.addEventListener("appmessage",
  function(e) {
    console.log("Message received from watch");

    if (typeof e.payload.fetchContactsKey !== "undefined") {
      fetchContacts();

    } else if (typeof e.payload.determineLocationKey !== "undefined") {
      isLocationDeterminationDemanded = true;
      window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);

    } else if (typeof e.payload.sendLocationKey !== "undefined") {
      contactIdToSendCoordinate = e.payload.sendLocationKey;
      sendCoordinateToContact();
    }
  }
);

//---------- Messages Sending to the watch --------

function sendMessageToWatch(message)
{
  prepareSendingMessageToWatch();
  doSendMessageToWatch(message);
}

function doSendMessageToWatch(message)
{
  Pebble.sendAppMessage(message,

    function(e) {
    },

    function(e) {
      console.log('Unable to deliver message, attempts counter = ' + messageSendingAttemptsLeft);

      if (isMessageSendingAttemptsLeft()) {
        setTimeout(function() {doSendMessageToWatch(message);}, messageSendingAttemptsInterval);
      }
    }
  );
}

function prepareSendingMessageToWatch()
{
  messageSendingAttemptsLeft = messageSendingAttemptsCount;
}

function isMessageSendingAttemptsLeft()
{
  if (messageSendingAttemptsLeft > 0) {
    messageSendingAttemptsLeft--;
    return true;
  }
  return false;
}

//---------- Http -------------------

function setupHttpRequest(req)
{
  //actually there are some issues in 'timeout' property support on pebble
  //http://forums.getpebble.com/discussion/13224/problem-with-xmlhttprequest-timeout
  req.timeout = requestTimeout;
}

function isHttpRequestSucceeded(req, resp)
{
  return req.readyState == 4 && req.status == 200 && typeof resp.errorKey === "undefined"
    && typeof resp.error === "undefined";
}

function isAuthRequired(req, resp)
{
  return req.status == 401 || resp.errorKey == 401;
}

//---------- Authorization --------------

function loadAccessToken()
{
  accessToken = window.localStorage.getItem("accessToken");
}

function storeAccessToken()
{
  if (accessToken != null && accessToken !== "undefined") {
    window.localStorage.setItem("accessToken", accessToken);

  } else {
    window.localStorage.removeItem("accessToken");
  }
}

function isAccessTokenValid()
{
  if (accessToken == null) {
    performAuthorization();
    return false;
  }

  return true;
}

function performAuthorization()
{
  if (accessToken != null) {
    accessToken = null;
    storeAccessToken();
  }

  sendMessageToWatch({"authorizationWorkflowKey": errorTypes.notAuthorized});
}

function login()
{
  Pebble.openURL('http://gdeseychas.ru/login?forward=pebble');
}

function onUserLogin(token)
{
 console.log("user logged in");
 accessToken = token;
 storeAccessToken();

 sendMessageToWatch({"authorizationWorkflowKey": errorTypes.noError});
}


function logout()
{
  Pebble.openURL('http://gdeseychas.ru/logout?forward=pebble');
}

function onUserLogout()
{
  performAuthorization();
}

//---------- Contacts ---------------

function fetchContacts()
{
  if (!isAccessTokenValid()) {
    return;
  }

  var req = new XMLHttpRequest();

  req.open('GET', 'http://gdeseychas.ru/api/v1.1/pebble/contacts?accessToken=' + accessToken, true);

  setupHttpRequest(req);

  req.onload = function(e) {
    var response = JSON.parse(req.responseText);

    if (isHttpRequestSucceeded(req, response)) {

        console.log("Contacts loaded, count = " + response.length);
        sendContactsToWatch(response);

    } else if (isAuthRequired(req, response)) {
      console.log("Unable to load contacts: authorization required");
      performAuthorization();

    } else {
      console.warn("Unable to load contacts");
      notifyFailedToLoadContacts(errorTypes.generalError);
    }
  }

  req.ontimeout = function(e) {
    console.warn("Unable to load contacts: timeout");
    notifyFailedToLoadContacts(errorTypes.httpTimeout);
  }

  req.send(null);
}

function sendContactsToWatch(contacts)
{
  prepareSendingMessageToWatch();
  doSendContactsToWatch(contacts);
}

function doSendContactsToWatch(contacts)
{
  Pebble.sendAppMessage({"contactsCountKey": contacts.length},
    function(e) {
      sendContactToWatch(contacts, 0);
    },
    function(e) {
      console.warn('Unable to deliver contacts count');

      if (isMessageSendingAttemptsLeft()) {
        console.log('Trying to send contacts count one more time...');
        setTimeout(function() {doSendContactsToWatch(contacts);}, messageSendingAttemptsInterval);
      }
    });
}

function sendContactToWatch(contacts,index)
{
  prepareSendingMessageToWatch();
  doSendContactToWatch(contacts, index);
}

function doSendContactToWatch(contacts, index)
{
  var contactName = contacts[index].name;

  if (contactName.length > maxContactNameLength) {
    contactName = contactName.substr(0, maxContactNameLength);
  }

  Pebble.sendAppMessage({"contactIdKey": contacts[index].id, "contactNameKey": contactName},
    function(e) {

      if (++index == contacts.length) {
        console.log("All contacts are deliverd to watch");

      } else {
        sendContactToWatch(contacts,index);
      }

    },
    function(e) {
      console.warn("Unable to deliver contact");

      if (isMessageSendingAttemptsLeft()) {
        console.log('Trying to send contact one more time...');
        setTimeout(function() {doSendContactToWatch(contacts, index);}, messageSendingAttemptsInterval);
      }
    });
}

function notifyFailedToLoadContacts(errorCode)
{
  sendMessageToWatch({"fetchContactsErrorKey": errorCode});
}

//---------- Sharing Location ---------------

function locationSuccess(pos) {
  lastCoordinate = pos.coords;
  locationError = null;

  //console.log('new user location received, accuracy = ' + lastCoordinate.accuracy + ', time = ' +
  //  pos.timestamp);


  if (isLocationDeterminationDemanded) {
    notifyLocationDetermined();
  }
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);

  locationError = err;

  if (isLocationDeterminationDemanded) {
    notifyFailedDetermineLocation();

  } else if (contactIdToSendCoordinate != null && !isLocationBeingSent) {
    notifyFailedToSendCoordinateToContact(errorTypes.locationIsNotDetermined, contactIdToSendCoordinate);
  }
}

function notifyLocationDetermined()
{
  var accuracy = lastCoordinate.accuracy | 0;

  if (accuracy > 10 && accuracy < 100) {
    accuracy = ((accuracy / 10) | 0) * 10;

  } else if (accuracy > 100) {
    accuracy = ((accuracy / 100) | 0) * 100;
  }

  sendMessageToWatch({"locationDeterminedKey": accuracy});
  isLocationDeterminationDemanded = false;
}

function notifyFailedDetermineLocation()
{
  sendMessageToWatch({"locationDeterminationErrorKey": errorTypes.locationIsNotDetermined});
  isLocationDeterminationDemanded = false;
}

function sendCoordinateToContact()
{
  if (!isAccessTokenValid()) {
    return;
  }

  if (lastCoordinate == null) {
    if (locationError != null) {
      notifyFailedToSendCoordinateToContact(errorTypes.locationIsNotDetermined, contactIdToSendCoordinate);
      return;

    } else {
      console.log("user location hasn't been determined yet");
      return;
    }
  }

  var req = new XMLHttpRequest();

  //IMPL_NOTE: deliberately copy current value, since it may be changed during the time request is being executed
  var contactId = contactIdToSendCoordinate;

  req.open('POST', 'http://gdeseychas.ru/api/v1/contacts/' + contactId + '/activity', true);


  var dataToPost = 'accessToken=' + accessToken + '&latitude=' + lastCoordinate.latitude +
                       '&longitude=' + lastCoordinate.longitude + '&radius=' + lastCoordinate.accuracy;

  dataToPost += '&comment=Sent%20from%20Pebble';

  //IMPL_NOTE: set these headers after req.open, otherwise POST won't work on Android
  req.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
  req.setRequestHeader("Content-Length", dataToPost.length);
  req.setRequestHeader("Connection", "close");

  setupHttpRequest(req);

  req.onload = function(e) {
    isLocationBeingSent = false;
    var response = JSON.parse(req.responseText);

    if (isHttpRequestSucceeded(req, response) && response.data !== "undefined") {

      console.log("Location is sent");
      sendMessageToWatch({"locationSentKey": contactId});
      contactIdToSendCoordinate = null;

    } else if (isAuthRequired(req, response)) {
      console.warn("Unable to send location: authorization required ");
      performAuthorization();

    } else {
      console.warn('Unable to send location: ' + req.responseText);
      notifyFailedToSendCoordinateToContact(errorTypes.generalError, contactId);
    }
  }

  req.ontimeout = function(e) {
    isLocationBeingSent = false;

    console.warn("Unable to sendLocation: timeout");
    notifyFailedToSendCoordinateToContact(errorTypes.httpTimeout, contactId);
  }

  isLocationBeingSent = true;
  req.send(dataToPost);
}

function notifyFailedToSendCoordinateToContact(errorCode, contactId)
{
  sendMessageToWatch({"locationSendingErrorKey": errorCode, "contactIdKey": contactId});
  contactIdToSendCoordinate = null;
}