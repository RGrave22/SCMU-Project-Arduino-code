const {onValueUpdated} = require("firebase-functions/v2/database");
const {log} = require("firebase-functions/logger");
const admin = require("firebase-admin");
admin.initializeApp();

exports.notifyTemperature = onValueUpdated({
  ref: "/greenhouses/{id}/temperature",
  region: "europe-west1",
},
async (event) => {
  // Get the parent ref (/greenhouses/{id})
  const greenhouseRef = event.data.after.ref.parent;
  const greenhouseSnapshot = await greenhouseRef.get();
  const greenhouseData = greenhouseSnapshot.val();

  if (!greenhouseRef) {
    log("No data found.");
    return null;
  }

  const greenhouseId = greenhouseData.id;
  if (!greenhouseId) {
    log("No greenhouse ID found in data.");
    return null;
  }

  const topic = `greenhouse_${greenhouseId}`; // e.g. "greenhouse_abc123"

  const temperature = greenhouseData.temperature;
  const tempThreshold = greenhouseData.thresholdTemperature;
  if (temperature > tempThreshold) {
    const payload = {
      notification: {
        title: "Temperature is too High!",
        body: "Temperature Level: " + temperature + "\u00B0C",
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

exports.notifyHumidity = onValueUpdated({
  ref: "/greenhouses/{id}/humidity",
  region: "europe-west1",
},
async (event) => {
  // Get the parent ref (/greenhouses/{id})
  const greenhouseRef = event.data.after.ref.parent;
  const greenhouseSnapshot = await greenhouseRef.get();
  const greenhouseData = greenhouseSnapshot.val();

  if (!greenhouseRef) {
    log("No data found.");
    return null;
  }

  const greenhouseId = greenhouseData.id;
  if (!greenhouseId) {
    log("No greenhouse ID found in data.");
    return null;
  }

  const topic = `greenhouse_${greenhouseId}`; // e.g. "greenhouse_abc123"

  const humidity = greenhouseData.humidity;
  const humidityThresholdMax = greenhouseData.thresholdHumidityMax;
  const humidityThresholdMin = greenhouseData.thresholdHumidityMin;

  if (humidity > humidityThresholdMax) {
    const payload = {
      notification: {
        title: "Humidity is too High!",
        body: "Humidity Level: " + humidity + "%",
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
  if (humidity < humidityThresholdMin) {
    const payload = {
      notification: {
        title: "Humidity is too Low!",
        body: "Humidity Level: " + humidity + "%",
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

exports.notifySoilHumidity = onValueUpdated({
  ref: "/greenhouses/{id}/soilHumidity",
  region: "europe-west1",
},
async (event) => {
  // Get the parent ref (/greenhouses/{id})
  const greenhouseRef = event.data.after.ref.parent;
  const greenhouseSnapshot = await greenhouseRef.get();
  const greenhouseData = greenhouseSnapshot.val();

  if (!greenhouseRef) {
    log("No data found.");
    return null;
  }

  const greenhouseId = greenhouseData.id;
  if (!greenhouseId) {
    log("No greenhouse ID found in data.");
    return null;
  }

  const topic = `greenhouse_${greenhouseId}`; // e.g. "greenhouse_abc123"

  const soilHumidity = greenhouseData.soilHumidity;
  const soilHumidityThreshold = greenhouseData.thresholdSoilHumidity;

  if (soilHumidity > soilHumidityThreshold) {
    const payload = {
      notification: {
        title: "Soil Humidity is too High!",
        body: "Soil Humidity Level: " + soilHumidity + "%",
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

exports.notifyLight = onValueUpdated({
  ref: "/greenhouses/{id}/light",
  region: "europe-west1",
},
async (event) => {
  // Get the parent ref (/greenhouses/{id})
  const greenhouseRef = event.data.after.ref.parent;
  const greenhouseSnapshot = await greenhouseRef.get();
  const greenhouseData = greenhouseSnapshot.val();

  if (!greenhouseRef) {
    log("No data found.");
    return null;
  }

  const greenhouseId = greenhouseData.id;
  if (!greenhouseId) {
    log("No greenhouse ID found in data.");
    return null;
  }

  const topic = `greenhouse_${greenhouseId}`; // e.g. "greenhouse_abc123"

  const light = greenhouseData.light;
  const lightThreshold = greenhouseData.thresholdLight;

  if (light > lightThreshold) {
    const payload = {
      notification: {
        title: "Light Levels are too High!",
        body: "Light Level: " + light + "%",
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
