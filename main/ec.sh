#!/bin/bash
echo "Starting Event Channel service..."
tao_cosevent -r -ORBEndpoint iiop://127.0.0.1:6555 -ORBInitRef NameService=corbaloc:iiop:127.0.0.1:9966/NameService -ORBSvcConf ec.conf
