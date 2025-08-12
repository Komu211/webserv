#!/usr/bin/env python3

import os
import sys

# --- Create the HTML body ---
body = """
<html>
<head><title>CGI Test</title></head>
<body>
    <h1>Hello from Python CGI!</h1>
    <h2>Environment Variables</h2>
    <ul>
"""

for key, value in os.environ.items():
    body += f"<li><b>{key}:</b> {value}</li>"

body += "</ul>"

# --- Check for POST data ---
if os.environ.get("REQUEST_METHOD") == "POST":
    body += "<h2>POST Data (from stdin)</h2><pre>"
    try:
        # Read from stdin, which the server pipes the request body to
        post_data = sys.stdin.read()
        body += post_data if post_data else "No POST data received."
    except Exception as e:
        body += f"Error reading stdin: {e}"
    body += "</pre>"

body += "</body></html>"

# --- Print the HTTP Response ---
# The CGI script's stdout is read by the web server.
# First, print the headers. A blank line separates headers from the body.
print("HTTP/1.1 200 OK", end="\r\n") # ! temporary line for testing. real CGI may not include this line
print("Content-Type: text/html", end="\r\n")
print(f"Content-Length: {len(body)}", end="\r\n")
print("Status: 200 OK", end="\r\n")
print(end="\r\n") # <-- blank line to signal start of body

# Now, print the body
print(body)
