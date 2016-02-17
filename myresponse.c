
char *response_404=
    "HTTP/1.0 404 NOT FOUND\n"
    "Content-type: text/html\n"
    "\n"
    "<html><head>"
    "<title>404 Not Found</title>"
    "</head><body>"
    "<h1>Not Found</h1>"
    "<p>The requested URL was not found on this server.</p>";


char *response_403=
    "HTTP/1.0 403 FORBIDDEN\n"
    "Content-type: text/html\n"
    "\n"
    "<html><head>"
    "<title>403 forbidden</title>"
    "</head><body>"
    "<h1>Forbidden</h1>"
    "<p>Access to the requested file is forbidden!</p>";


char *response_200 =
    "HTTP/1.0 200 OK\n";

