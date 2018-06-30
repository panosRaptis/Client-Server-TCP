# Client-Server-TCP

The following program is devided into the following 3 units:
 - Web Site Creator
 - TCP Web Server
 - TCP Web Crawler
 
### Web Site Creator in Shell

Creates w websites which contain p pages (html files) - on disk and store them into root_directory.

##### Running
```
./webcreator.sh root_directory text_file.txt w p
```
### TCP Web Server in C

Serves HTTP requests from specific websites' pages -by searching into root_directory- and returns the appropriate HTTP responce (HTTP 200, 400, 403, 404). A threadpool is used in order to manage all incoming requests. Each thread is responsible for one request.
Server can also listen to an other port (command port) in odrer to receive the following command requests from the administrator:
 * STATS: Implements time up, number of pages served and number of bytes read.
 * SHUTDOWN: Closes connection and terminates.
Administrator can connect to command port using *eg. Telnet or ncat etc*.

##### Compilation
```
make
```
##### Running
```
./myhttpd -p serverport -c commandport -t num_threads -d root_directory
```
### TCP Web Crawler in C

Starts from a starting URL. If this URL is valid, it searches recursevly for other URLs - into that page - and sends a GET request for each of them. Process completes after all links from each page are downloaded. A threadpool is also used in order to manage all responces. Each thread is responsible for one responce.
Crawler can also listen to an other port (command port) in odrer to receive the following command requests from the administrator:
 * STATS: Implements time up, number of pages served and number of bytes written.
 * SHUTDOWN: Closes connection and terminates.
 * SEARCH words: Searches for those words (max. 10) into downloaded pages, ignoring HTML Tags. Search Command is implemanted by generating new worker processes. Crawler connects with each of these processes using pipes.  

Administrator can connect to command port using *eg. Telnet or ncat etc*.

##### Compilation
```
make
```
##### Running
```
./mycrawler -h host_or_IP_Server -p serverport -c commandport -t num_threads -d save_dir starting_URL
```
 
 


