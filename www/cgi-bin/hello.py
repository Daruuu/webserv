#!/usr/bin/env python3
"""
Simple CGI test - outputs hello
"""
import os

print("Content-Type: text/plain")
print("")
print("Hello from CGI!")
print(f"You requested: {os.environ.get('REQUEST_URI', 'unknown')}")
print(f"Method: {os.environ.get('REQUEST_METHOD', 'unknown')}")
