#!/bin/bash
echo "Starting Naming Service..."
tao_cosnaming -f state.dat -ORBEndpoint iiop://127.0.0.1:9966
