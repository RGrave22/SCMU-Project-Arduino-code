const {onValueUpdated} = require("firebase-functions/v2/database");
const {
  log,
} = require("firebase-functions/logger");
const admin = require("firebase-admin");
admin.initializeApp();

exports.notifyChanges = onValueUpdated({
  ref: "/greenhouses/{owner}",
  region: "europe-west1",
},
async (event) => {
  const greenhouseData = event.data.after.val();

  const userId = greenhouseData.owner;
  log("USER ID: " + userId);
  let fcmToken;
  try {
    const tokenSnapshot = await admin
        .database()
        .ref(`/users/${userId}/fcmToken`).once("value");
    fcmToken = tokenSnapshot.val();
  } catch (error) {
    console.error(`Failed to retrieve FCM token for user ${userId}:`, error);
    return null;
  }

  if (!fcmToken) {
    log(`No FCM token for user ${userId}`);
    return null;
  }
  log("FCM TOKEN: " + fcmToken);

  const payload = {
    notification: {
      title: "Greenhouse Active",
      body: "Your greenhouse status is now active!",
    },
    token: fcmToken,
  };

  try {
    const response = await admin.messaging().send(payload);
    log(`Notification sent to ${userId}:`, response);
  } catch (error) {
    console.error("Error sending notification:", error);
  }

  return null;
});
