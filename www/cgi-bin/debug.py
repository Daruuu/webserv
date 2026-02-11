#!/usr/bin/env python3
import sys
import os

sys.stderr.write("=== CGI STARTED ===\n")
sys.stderr.write(f"REQUEST_METHOD={os.environ.get('REQUEST_METHOD', 'NONE')}\n")
sys.stderr.write(f"SCRIPT_FILENAME={os.environ.get('SCRIPT_FILENAME', 'NONE')}\n")
sys.stderr.flush()

print("Status: 200 OK")
print("Content-Type: text/plain")
print("")
print("Hello from debug CGI!")
print(f"Method: {os.environ.get('REQUEST_METHOD', 'Unknown')}")
sys.stderr.write("=== CGI ENDING ===\n")
sys.stderr.flush()
