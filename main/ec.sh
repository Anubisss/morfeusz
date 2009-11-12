#!/bin/bash
echo "Starting Event Channel service..."
CosEvent_Service -r  -ORBEndpoint iiop://localhost:6555 -ORBInitRef NameService=corbaloc:iiop:localhost:9966/NameService -ORBSvcConf ec.conf