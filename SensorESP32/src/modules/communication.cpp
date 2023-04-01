/**
 * This module handles network communication with the remote webserver.
 * 
 * @authors Kyle T., Ashraf K., Thomas M.
*/

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-mysql-database-php/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include "communication.h"
#include "config.h"
#include "debug.h"
#include "Arduino.h"

#include <WiFi.h>
#include <HTTPClient.h>

HTTPClient http;   
WiFiClient client;

/**
 * Setup method called by the main task to initialise any resources needed by
 * the communication subsystem.
*/
void setupCommunication() 
{
    Serial.begin(115200);
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    dbg_log_info("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(1000);
        dbg_log_info(".");
    }
    dbg_log_info("");

    dbg_log_info("Connected to WiFi network with IP Address: ");
    dbg_log_info("%s", WiFi.localIP().toString());
}

/**
 * This method is called by the main task to send the data to the server.
 * 
 * @param msg the message to send the server.
 */
void sendData(String msg) 
{
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) 
    {
        // Your Domain name with URL path or IP address with path
        http.begin(API_SERVER_URL);

        // Connect to the server
        client.connect(API_SERVER_URL, 80);

        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        // Prepare your HTTP POST request data
        String httpRequestData = "api_key=" API_KEY "&sensor=" API_SENSOR_NAME "&value=" + msg;

        //print the httpRequestData to display
        // dbg_log_info("httpRequestData: ");
        // dbg_log_info("%s", httpRequestData.c_str());

        // Send HTTP POST request
        int httpResponseCode = http.POST(httpRequestData);

        if (httpResponseCode == HTTP_CODE_OK) 
        {
            String serverResponse = http.getString();

            //print the serverResponse to display
            dbg_log_info("Sent data to server! Server response: ");
            dbg_log_info("  %s", serverResponse.c_str());
        }
        else 
        {
            //print the httpResponseCode to display
            dbg_log_warning("HTTP failed, error code: %s", std::to_string(httpResponseCode).c_str());
        }

        // Free resources
        http.end();
    }
    else 
    {
        dbg_log_warning("WiFi Disconnected");
    }
}
