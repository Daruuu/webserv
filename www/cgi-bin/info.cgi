#!/usr/bin/env python3
"""
Executable CGI script (using .cgi extension)
Demonstrates direct execution via shebang
Returns system and request information in JSON format
"""
import os
import sys
import json
from datetime import datetime

# Collect environment data
env_data = {
    'cgi_version': os.environ.get('GATEWAY_INTERFACE', 'N/A'),
    'server_software': os.environ.get('SERVER_SOFTWARE', 'N/A'),
    'server_protocol': os.environ.get('SERVER_PROTOCOL', 'N/A'),
    'request_method': os.environ.get('REQUEST_METHOD', 'N/A'),
    'query_string': os.environ.get('QUERY_STRING', ''),
    'script_name': os.environ.get('SCRIPT_NAME', 'N/A'),
    'script_filename': os.environ.get('SCRIPT_FILENAME', 'N/A'),
    'request_uri': os.environ.get('REQUEST_URI', 'N/A'),
    'content_type': os.environ.get('CONTENT_TYPE', ''),
    'content_length': os.environ.get('CONTENT_LENGTH', '0'),
    'remote_addr': os.environ.get('REMOTE_ADDR', 'N/A'),
}

# Get all HTTP headers
http_headers = {}
for key, value in os.environ.items():
    if key.startswith('HTTP_'):
        header_name = key[5:].replace('_', '-').title()
        http_headers[header_name] = value

# Collect system info
system_info = {
    'python_version': sys.version,
    'platform': sys.platform,
    'timestamp': datetime.now().isoformat(),
    'pid': os.getpid(),
}

# Build response data
response_data = {
    'status': 'success',
    'message': 'CGI script executed successfully via .cgi extension',
    'environment': env_data,
    'http_headers': http_headers,
    'system': system_info,
}

# Output HTTP headers
print("Content-Type: application/json")
print("Status: 200 OK")
print()

# Output JSON response
print(json.dumps(response_data, indent=2))
