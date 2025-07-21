# Webserv – HTTP 1.1 Non-Blocking Web Server

**Webserv** is a lightweight, HTTP 1.1-compliant web server built from scratch in C++. 
It is designed to handle multiple clients concurrently using a single-threaded, non-blocking I/O model via `poll()`. 
The server supports multiple ports, configurable routing, CGI execution, and basic file uploads, 
making it suitable for serving static websites or acting as a learning tool for low-level web server implementation.

---

## 🔧 Features

- ✅ Fully non-blocking architecture using `poll()` for all I/O operations  
- ✅ Single `poll()` call for all file descriptors (including `listen`)  
- ✅ MacOS-compatible non-blocking I/O with `fcntl()` (`O_NONBLOCK` only)  
- ✅ Accurate HTTP status codes with default error pages  
- ✅ Supports **GET**, **POST**, and **DELETE** methods  
- ✅ Static file serving & file uploads  
- ✅ CGI support (e.g., Python, PHP) using `fork()`  
- ✅ Route configuration: allowed methods, redirection, root path, autoindex  
- ✅ Multi-port listening & multiple virtual server blocks  
- ✅ Configuration inspired by NGINX server blocks  
- ✅ Compatible with modern web browsers  
- ✅ Graceful error handling and response timeouts  
- ✅ Stress-tested to stay available under high load  

---

## ⚙️ Usage

Compile the server:
```bash
make
```

Run with:
```bash
./webserv path/to/config.conf
```

