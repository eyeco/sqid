# sqid Plugin for MQTT

_TODO: general description_

## dependencies

- Eclipse Mosquitto MQTT (tested with 2.0.14, 64 bit) [[link](https://mosquitto.org/)]

## List of operators

- networking
	- mqttIn
	- mqttOut

# TODO

- implement multithreading; move peers' (currently blocking) connection attempt away from main thread
- implement TLS connection
