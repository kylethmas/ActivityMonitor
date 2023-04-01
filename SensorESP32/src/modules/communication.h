#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>

/**
 * This module handles network communication with the remote webserver.
 * 
 * @authors 
*/

/**
 * Setup method called by the main task to initialise any resources needed by
 * the communication subsystem.
*/
void setupCommunication(); 

/**
 * This method is called by the main task to send the data to the server.
 * 
 * @param msg the message to send the server.
 */
void sendData(String msg);

#endif // COMMUNICATION_H