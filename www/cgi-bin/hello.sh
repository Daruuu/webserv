#!/bin/bash
# Simple Bash CGI script to test shell script execution
# Returns environment variables and request information

echo "Content-Type: text/html"
echo "Status: 200 OK"
echo ""
echo "<!DOCTYPE html>"
echo "<html><head><title>Bash CGI Test</title></head>"
echo "<body>"
echo "<h1>Bash CGI Script - Environment Variables</h1>"
echo "<p><strong>Request Method:</strong> $REQUEST_METHOD</p>"
echo "<p><strong>Query String:</strong> $QUERY_STRING</p>"
echo "<p><strong>Script Name:</strong> $SCRIPT_NAME</p>"
echo "<p><strong>Script Filename:</strong> $SCRIPT_FILENAME</p>"
echo "<p><strong>Server Protocol:</strong> $SERVER_PROTOCOL</p>"
echo "<p><strong>Server Software:</strong> $SERVER_SOFTWARE</p>"
echo "<p><strong>Gateway Interface:</strong> $GATEWAY_INTERFACE</p>"
echo "<p><strong>Remote Address:</strong> $REMOTE_ADDR</p>"
echo "<p><strong>Content Length:</strong> $CONTENT_LENGTH</p>"
echo "<p><strong>Content Type:</strong> $CONTENT_TYPE</p>"
echo "<hr>"
echo "<h2>All Environment Variables:</h2>"
echo "<pre>"
env | sort
echo "</pre>"
echo "</body></html>"
