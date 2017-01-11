Just a quick guide to give you an idea of how this works. This file will be updated in order to make it more clear in the future.

Arduino uses NRF24L01 radiomodule for communication, FC-28 soil moisture sensor, DHT-11 sensor and HD44730 compliant LCD display. It also steers a water pump through PWM.

Th important part of this is:

unsigned long pump_on_delay;
unsigned long pump_inactive_delay;

These two variables are read on setup of the arduino device in read_setup() function. Values are sent in using following protocol.

<h1>Communication protocol</h1>

THe protocol is as follows:
Arduino device is in listening state. When it receives a message it interprets it and sends back acknowledgment, which can be requested data.

Arduino device can be used in two ways:
<ul>
<li> sending current state,
<li> receiving configuration.
</ul>

Receiving current state basically means sending back HUMID/DRY message back to requesting device. GETSTATE string must be sent in order to make arduino send back its state.
Receiving configuration means Arduino. Currently it means receiving SETPUMPxxxxyyyy string, where x's are interpreted as 4-digit base-10 time interval for pump_on_delay variable and y's 4-digit base-10 time interval for pump_inactive_delay variable.

If any other string is sent it is interpreted as SETPUMP request. This means device is totally insecure, but "done is better than perfect". It meets basic needs, so this issue will be dealt with later.
