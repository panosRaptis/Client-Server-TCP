# Client-Server-TCP

The following program is devided into the following 3 units:
 - Web Site Creator
 - TCP Web Server
 - TCP Web Crawler
 
### Web Site Creator in Shell

Creates w websites which contain p pages (html files) - on disk and store them into root_directory.

##### Compilation
```
./webcreator.sh root_directory text_file.txt w p
```
### TCP Web Server in C

Serves HTTP requests from specific websites' pages -by searching into root_directory- and returns the appropriate HTTP responce (HTTP 200, 400, 403, 404). A threadpool is used in order to manage all incoming requests. Each thread is responsible for one request.
Server can also listen to other ports in odrer to receive the following command requests from the administrator:
 * STATS: Implements time up, number of pages served and number of bytes read.
 * SHUTDOWN: Closes Connection
 
 


