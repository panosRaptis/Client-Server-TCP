# Client-Server-TCP

The following program is devided into the following 3 units:
 - Web Site Creator
 - TCP Web Server
 - TCP Web Client (Crawler)
 
### Web Site Creator in Shell

Creates w websites which contain p pages (html files) - on disk and store them into root_directory.

##### Running
```
./webcreator.sh root_directory text_file.txt w p
```
### TCP Web Server in C

Serves HTTP requests from specific websites' pages -by searching into root_directory- and returns the appropriate HTTP responce (HTTP 200, 400, 403, 404). A threadpool (with num_threads) is used in order to manage all incoming requests. Each thread is responsible for one request.
Server can also listen to an other port (command port) in odrer to receive the following command requests from the administrator:
 * STATS: Implements time up, number of pages served and number of bytes read.
 * SHUTDOWN: Closes connection and terminates.
 
Connecting to command port can be done using *Telnet*.

##### Compilation
```
make
```
##### Running
```
./myhttpd -p serverport -c commandport -t num_threads -d root_directory
```
### TCP Web Client (Crawler) in C

Starts from a starting_URL. If this URL is valid, it searches recursevly for other URLs, into that page, and sends a GET request for each of them. Process completes after all links from each page are downloaded. A threadpool (with num_threads) is also used in order to manage all responces. Each thread is responsible for one responce.
Crawler can also listen to an other port (command port) in odrer to receive the following command requests from the administrator:
 * STATS: Implements time up, number of pages served and number of bytes written.
 * SHUTDOWN: Closes connection and terminates.
 * SEARCH word1 [word2 ... word10]: Searches for those words (max. 10) into downloaded pages, ignoring HTML Tags. Search Command is implemanted by generating new worker processes. Crawler connects with each of these processes using pipes.  

Connecting to command port can be done using *Telnet*.

##### Compilation
```
make
```
##### Running
```
./mycrawler -h server_host_or_IP -p server_port -c crawler_command_port -t num_threads -d save_dir starting_URL
```
 
 


