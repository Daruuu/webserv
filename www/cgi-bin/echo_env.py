#!/usr/bin/env python3
"""
Echo all CGI environment variables
Useful for testing CGI environment setup
"""
import os
import sys

print("Content-Type: text/plain")
print("")

# Print selected important environment variables
important_vars = [
    'REQUEST_METHOD',
    'QUERY_STRING',
    'PATH_INFO',
    'PATH_TRANSLATED',
    'CONTENT_LENGTH',
    'CONTENT_TYPE',
    'GATEWAY_INTERFACE',
    'SERVER_PROTOCOL',
    'SERVER_SOFTWARE',
    'SERVER_NAME',
    'SERVER_PORT',
    'SCRIPT_NAME',
    'SCRIPT_FILENAME',
    'REQUEST_URI',
    'REMOTE_ADDR'
]

print("=== Important CGI Variables ===")
for var in important_vars:
    value = os.environ.get(var, '')
    print(f"{var}={value}")

print("\n=== HTTP Headers (HTTP_* variables) ===")
for key in sorted(os.environ.keys()):
    if key.startswith('HTTP_'):
        print(f"{key}={os.environ[key]}")

print("\n=== All Environment Variables ===")
for key in sorted(os.environ.keys()):
    print(f"{key}={os.environ[key]}")
