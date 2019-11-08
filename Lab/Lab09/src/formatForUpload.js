var payload = msg.payload
payload = payload.substring(0,4);

msg.payload = JSON.stringify([{"temperature_sensor":payload}])

return msg;