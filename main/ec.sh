#!/bin/bash
echo "Starting Event Channel service..."
CosEvent_Service -x -ORBEndPoint iiop://localhost:6555 -ORBSvcConf ec.conf