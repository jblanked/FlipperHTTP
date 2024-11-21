# Usage
This folder contains the source code for developer use in Flipper Zero apps and scripts written in C, JavaScript, and MicroPython.

## Usage in `C` (flipper_http.h and flipper_http.c)

| **Function Name**                                | **Return Value** | **Parameters**                                                                                                | **Description**                                                                                   |
|--------------------------------------------------|------------------|----------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------|
| `flipper_http_init`                              | `bool`           | `FlipperHTTP_Callback callback`, `void *context`                                                               | Initializes the HTTP module with a callback and context.                                           |
| `flipper_http_deinit`                            | `void`           | None                                                                                                           | Deinitializes the HTTP module.                                                                    |
| `flipper_http_connect_wifi`                      | `bool`           | None                                                                                                           | Connects to WiFi using previously saved credentials.                                               |
| `flipper_http_disconnect_wifi`                   | `bool`           | None                                                                                                           | Disconnects from the current WiFi network.                                                         |
| `flipper_http_ping`                              | `bool`           | None                                                                                                           | Sends a ping request to test connectivity.                                                         |
| `flipper_http_scan_wifi`                         | `bool`           | `const char *ssid`, `const char *password`                                                                     | Scans for nearby WiFi networks.                                                                    |
| `flipper_http_save_wifi`                         | `bool`           | `const char *ssid`, `const char *password`                                                                     | Saves WiFi credentials for future connections.                                                     |
| `flipper_http_ip_wifi`                           | `bool`           | None                                                                                                           | Retrieves the IP address of the connected WiFi network.                                            |
| `flipper_http_ip_address`                        | `bool`           | None                                                                                                           | Retrieves the IP address of the WiFi Devboard.                                                     |
| `flipper_http_list_commands`                     | `bool`           | None                                                                                                           | Lists all available commands.                                                                      |
| `flipper_http_led_on`                            | `bool`           | None                                                                                                           | Allow the LED to display while processing.                                                         |
| `flipper_http_led_off`                           | `bool`           | None                                                                                                           | Disable the LED from displaying while processing.                                                  |
| `flipper_http_parse_json`                        | `bool`           | `const char *key`, `const char *json_data`                                                                     | Parses JSON data for a specified key.                                                              |
| `flipper_http_parse_json_array`                  | `bool`           | `const char *key`, `int index`, `const char *json_data`                                                        | Parses an array within JSON data for a specified key and index.                                    |
| `flipper_http_send_data`                         | `bool`           | `const char *data`                                                                                             | Sends the specified data to the server.                                                            |
| `flipper_http_rx_callback`                       | `void`           | `const char *line`, `void *context`                                                                            | Callback function for handling received data.                                                      |
| `flipper_http_get_request`                       | `bool`           | `const char *url`                                                                                              | Sends a GET request to the specified URL.                                                          |
| `flipper_http_get_request_with_headers`          | `bool`           | `const char *url`, `const char *headers`                                                                       | Sends a GET request with custom headers to the specified URL.                                      |
| `flipper_http_post_request_with_headers`         | `bool`           | `const char *url`, `const char *headers`, `const char *payload`                                                | Sends a POST request with custom headers and a payload to the specified URL.                       |
| `flipper_http_put_request_with_headers`          | `bool`           | `const char *url`, `const char *headers`, `const char *payload`                                                | Sends a PUT request with custom headers and a payload to the specified URL.                        |
| `flipper_http_delete_request_with_headers`       | `bool`           | `const char *url`, `const char *headers`, `const char *payload`                                                | Sends a DELETE request with custom headers and a payload to the specified URL.                     |

`In C, fhttp.last_response holds the received data. In JavaScript and mPython, the response is returned directly from the function.`

## Usage in `JavaScript` (flipper_http.js):
| **Function Name**                      | **Return Value** | **Parameters**                                       | **Description**                                                                                      |
|----------------------------------------|------------------|-----------------------------------------------------|------------------------------------------------------------------------------------------------------|
| `fhttp.init`                           | `void`           | None                                                | Initializes the serial connection with the correct settings.                                          |
| `fhttp.deinit`                         | `void`           | None                                                | Deinitializes and ends the serial connection.                                                         |
| `fhttp.connect_wifi`                   | `bool`           | None                                                | Sends a command to connect to WiFi and returns whether the connection was successful.                 |
| `fhttp.disconnect_wifi`                | `bool`           | None                                                | Sends a command to disconnect from WiFi and returns whether the disconnection was successful.          |
| `fhttp.ping`                           | `bool`           | None                                                | Sends a ping request to test connectivity and returns whether a response was received.                |
| `fhttp.scan_wifi`                      | `string`         | None                                                | Scans for nearby WiFi access points and returns a string with each access point separated by a comma.  |
| `fhttp.save_wifi`                      | `bool`           | `ssid: string`, `password: string`                  | Saves WiFi credentials and returns whether the save operation was successful.                         |
| `fhttp.ip_wifi`                        | `string`         | None                                                | Gets the IP address of the connected WiFi network and returns it as a string.                      |
| `fhttp.ip_address`                     | `string`         | None                                                | Gets the IP address of the WiFi Devboard and returns it as a string.                                  |
| `fhttp.list_commands`                  | `string`         | None                                                | Lists all available commands and returns them as a string.                                            |
| `fhttp.led_on`                         | `void`           | None                                                | Sends a command to allow the LED to display while processing.                                         |
| `fhttp.led_off`                        | `void`           | None                                                | Sends a command to disable the LED from displaying while processing.                                  |
| `fhttp.parse_json`                     | `string`         | `key: string`, `data: string`                       | Parses JSON data for a specified key and returns the corresponding value as a string.                 |
| `fhttp.parse_json_array`               | `string`         | `key: string`, `index: number`, `data: string`      | Parses an array within JSON data for a specified key and index, returning the corresponding value.    |
| `fhttp.send_data`                      | `void`           | `data: string`                                      | Sends the specified data to the serial port.                                                          |
| `fhttp.read_data`                      | `string`         | `delay_ms: number`                                  | Reads data from the serial port with a specified delay and returns the response received.             |
| `fhttp.get_request`                    | `string`         | `url: string`                                       | Sends a GET request to the specified URL and returns the response.                                    |
| `fhttp.get_request_with_headers`       | `string`         | `url: string`, `headers: string`                    | Sends a GET request with specified headers and returns the response.                                  |
| `fhttp.post_request_with_headers`      | `string`         | `url: string`, `headers: string`, `payload: string` | Sends a POST request with specified headers and payload, returning the response.                      |
| `fhttp.put_request_with_headers`       | `string`         | `url: string`, `headers: string`, `payload: string` | Sends a PUT request with specified headers and payload, returning the response.                       |
| `fhttp.delete_request_with_headers`    | `string`         | `url: string`, `headers: string`, `payload: string` | Sends a DELETE request with specified headers and payload, returning the response.                    |

## Usage in `Python` (flipper_http.py):
| **Function Name**                      | **Return Value** | **Parameters**                                       | **Description**                                                                                      |
|----------------------------------------|------------------|-----------------------------------------------------|------------------------------------------------------------------------------------------------------|
| `flipper_http_connect_wifi`                   | `bool`           | None                                                | Sends a command to connect to WiFi and returns whether the connection was successful.                 |
| `flipper_http_disconnect_wifi`                | `bool`           | None                                                | Sends a command to disconnect from WiFi and returns whether the disconnection was successful.          |
| `flipper_http_ping`                           | `bool`           | None                                                | Sends a ping request to test connectivity and returns whether a response was received.                |
| `flipper_http_save_wifi`                      | `bool`           | `ssid: str`, `password: str`                  | Saves WiFi credentials and returns whether the save operation was successful.                         |
| `flipper_http_scan_wifi`                      | `str`            | None                                          | Scans for nearby WiFi access points and returns a string listing each access point, separated by commas.  |
| `flipper_http_ip_wifi`                        | `str`            | None                                          | Gets the IP address of the connected WiFi network and returns it as a string.                       |
| `flipper_http_ip_address`                     | `str`            | None                                          | Gets the IP address of the WiFi Devboard and returns it as a string.                                  |
| `flipper_http_list_commands`                  | `str`            | None                                          | Lists all available commands and returns them as a string.                                            |
| `flipper_http_led_on`                         | `void`           | None                                          | Sends a command to allow the LED to display while processing.                                         |
| `flipper_http_led_off`                        | `void`           | None                                          | Sends a command to disable the LED from displaying while processing.                                  |
| `flipper_http_parse_json`                     | `str`            | `key: str`, `json_data: str`                   | Parses JSON data for a specified key and returns the corresponding value as a string.                 |
| `flipper_http_parse_json_array`               | `str`            | `key: str`, `index: int`, `json_data: str`       | Parses an array within JSON data for a specified key and index, returning the corresponding value.    |
| `flipper_http_send_data`                      | `void`           | `data: str`                                      | Sends the specified data to the serial port.                                                          |
| `flipper_http_read_data`                      | `str`            | `sleep_ms: int`                                  | Reads data from the serial port with a specified delay and returns the response received.             |
| `flipper_http_get_request`                    | `str`            | `url: str`                                       | Sends a GET request to the specified URL and returns the response.                                    |
| `flipper_http_get_request_with_headers`       | `str`            | `url: str`, `headers: str`                      | Sends a GET request with specified headers and returns the response.                                  |
| `flipper_http_post_request_with_headers`      | `str`            | `url: str`, `headers: str`, `data: str`         | Sends a POST request with specified headers and data, returning the response.                         |
| `flipper_http_put_request_with_headers`       | `str`            | `url: str`, `headers: str`, `data: str`         | Sends a PUT request with specified headers and data, returning the response.                          |
| `flipper_http_delete_request_with_headers`    | `str`            | `url: str`, `headers: str`, `data: str`         | Sends a DELETE request with specified headers and data, returning the response.                       |