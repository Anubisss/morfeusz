#!/bin/bash
echo "Starting Naming Service..."
Naming_Service -f state.dat -ORBEndpoint iiop://127.0.0.1:9966 -m 1
