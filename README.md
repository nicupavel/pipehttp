# Pipe Http
*Small and high performance pipe input to http output with persistent in-memory circular buffer with no libraries dependencies.*

## What it does ##
 Takes the output from another application and makes it available on HTTP. The output is kept in memory in a circular buffer
 (old entries above a set limit are deleted) and made available thought a REST like HTTP API. Application output can  be rate limited 
 (eg: don't save data that comes too fast) and modified by adding strings as prefix/suffix and lines delimiter.
 Multiple pipes can be combined with different rates and settings (see [Examples](# Examples)).

## Running

    # your_app | ph 

Point the browser to *localhost:8000* to see *app* output
## REST API

By default ph will bind to port 8000 and * address. URLs that are known:
- **GET /** - returns the entire memory buffer
- **GET /1** - returns the most recent line/block
- **GET /n** - returns the specified line/block number
- **GET /clear** - clears the entire memory buffer
- **GET /config?rate=60&max_lines=100** - dynamically changes the running configuration. In this case it will set rate limiting to 1 message every minute and maximum lines on circular buffer to 100. 

Known **GET /config** options:
    - **rate** - rate limiting
    - **max_lines** - size of circular buffer in blocks or lines
    - **timeout** - set the inactivity timeout -1 means forever
    - **output_stdin** - 0 disables 1 enables output of received data to stdout
## Running Options

    -a <addr>       - The address to bind. Default any
    -p <port>       - The port to bind. Default 8000"
    -l <number>     - Max number of lines to hold. Default "
    -t <seconds>    - Inactivity timeout in seconds. Default infinite
    -b <string>     - String to append at the begining of response. Default none.
    -s <string>     - String to append at end of response. Default none.
    -d <string>     - Line delimiter string to append between lines (except last line).Default none.
    -r <seconds>    - Rate limiting incoming lines. Lines comming faster will be ignored.Default no limit.
    -o              - Don't output stdin to stdout
    -h              - This help.
    -V              - Display version information and exit.

## Building
For building make and gcc are needed. Default installation root is ```usr/local``` that can be changed by exporting environment variable ROOT_PREFIX before doing ```make install```

    cd pipehttp
    make
    sudo make install

## Caveats
Applications might not flush stdout so their output might not be visible to *ph* imediatelly. For example in grep case use ```grep --line-buffered``` to fix this. Also applications might dump multiple lines of text at once which *ph* will see it as a single line or block. When changing ```stdin_output``` dinamically using the REST API the next *ph* commands in a pipe chain will no longer get output from the modified *ph* instance. This might *be or not be* what you intended.

## Examples
- Continously run a program and save last 10 output lines to buffer:

    ```# cat /dev/zero | xargs --null sh -c "sudo hddtemp /dev/sda /dev/sdb ; sleep 5" | ph -l 10 ```

- Monitor and keep last 2000 of log entries
    ```# journalctl -f | ph -l 2000```

- Monitor a file and keep only 100 lines: 
    ```# tail -f /var/log/syslog | ph -l 100```

- Pipe multiple *ph* commands with different rates and buffer size. With command below 100k lines will be held available on port 8000 with 1 message per second and only plugdev messages will be shown on port 8001 instance at a rate of 1 per minute:

    ```# journalctl -f |  ph -l 100000 -r 1 | grep --line-buffered plugdev | ph -l 100 -r 60 -p 8001```

- If the app is outputing a json (eg: ```{"temperature_C": 24, "humidity": 47}```) you can get the entire buffer as json:

    ```# rtl_sdr -f 915M -F json | ph -l 5000 -r 60 -d , -b [ -s ]```
## Similarity
- If you don't need persistence (client access consumes buffer) or circular buffering  and rate limiting netcat/socat is a good alternative:
    - Server: 
        ```# mkfifo mypipe```
        ```# nc -l -p port < mypipe  | your_app > mypipe```
    - Client:
        ```nc server port```

- If you need to parse log data using patterns to make it queryable with HTTP use something like [grok_exporter](https://github.com/fstab/grok_exporter)