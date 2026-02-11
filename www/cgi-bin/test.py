#!/usr/bin/python3
import os
import sys
import time

print("Status: 200 OK")
print("Content-Type: text/plain")
print("")
print("Hello from Python CGI!")
print("Environment Variables:")
time.sleep(4)
for key, value in os.environ.items():
    print(f"{key}={value}")

if os.environ.get("REQUEST_METHOD") == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    if content_length > 0:
        body = sys.stdin.read(content_length)
        print("\nRequest Body:")
        print(body)
