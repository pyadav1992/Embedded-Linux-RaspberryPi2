This is a demonstration of socket prograaming on a target linux-kernel based board (Raspberry-Pi2):
- A thermostat cooling stage is implemented that turns on the cooler when the temperature goes above a set point and an alarm is alerted when the temperature goes beyond a limit.
- A server is implemented that creates a server socket which is used to monitor parameters for the thermostat (setpoint, limit, deadband).
- A client is implemented that creates a client socket which is used to pass the thermostat parameters to the servers.
- Multithreading is used to demonstrate multiple client network that try to communicate with the thermostat server.
