#!/bin/bash
printf "Status: 200 OK\r\n"
printf "Content-Type: text/plain\r\n\r\n"
echo "Hello from Bash CGI!"
echo "Environment Variables:"
env

if [ "$REQUEST_METHOD" = "POST" ]; then
    echo ""
    echo "Request Body:"
    cat
fi
