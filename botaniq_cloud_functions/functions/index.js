const {onValueUpdated} = require("firebase-functions/v2/database");
const {log} = require("firebase-functions/logger");
const admin = require("firebase-admin");
admin.initializeApp();

exports.notifyChanges = onValueUpdated({
  ref: "/greenhouses/{owner}",
  region: "europe-west1",
},
async (event) => {
  const greenhouseData = event.data.after.val();

  if (!greenhouseData) {
    log("No greenhouse data found.");
    return null;
  }

  const greenhouseId = greenhouseData.id;
  if (!greenhouseId) {
    log("No greenhouse ID found in data.");
    return null;
  }

  const topic = `greenhouse_${greenhouseId}`; // e.g. "greenhouse_abc123"

  const payload = {
    notification: {
      title: "Greenhouse Active",
      body: "Your greenhouse status is now active!",
    },
    topic: topic,
  };

  try {
    const response = await admin.messaging().send(payload);
    log(`Notification sent to topic "${topic}":`, response);
  } catch (error) {
    console.error("Error sending notification to topic:", error);
  }

  return null;
},
);

exports.notifyTemperature = onValueUpdated({
  ref: "/greenhouses/{id}",
  region: "europe-west1",
},
async (event) => {
  const greenhouseData = event.data.after.val();

  if (!greenhouseData) {
    log("No greenhouse data found.");
    return null;
  }

  const greenhouseId = greenhouseData.id;
  if (!greenhouseId) {
    log("No greenhouse ID found in data.");
    return null;
  }

  const topic = `greenhouse_${greenhouseId}`; // e.g. "greenhouse_abc123"

  const temperature = greenhouseData.id.temperature;
  const tempThreshold = greenhouseData.id.thresholds.maxTemperature;
  if (temperature > tempThreshold) {
    const payload = {
      notification: {
        title: "Temperature is too High!",
        body: "Temperature Level: " + temperature,
      },
      topic: topic,
    };

    try {
      const response = await admin.messaging().send(payload);
      log(`Notification sent to topic "${topic}":`, response);
    } catch (error) {
      console.error("Error sending notification to topic:", error);
    }
  }
  return null;
},
);
