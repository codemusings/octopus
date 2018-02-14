#!/bin/sh
curl -X PUT -u admin:admin -d '"true"' localhost:5984/_node/nonode@nohost/_config/couch_peruser/delete_dbs
curl -X PUT -u admin:admin -d '"true"' localhost:5984/_node/nonode@nohost/_config/couch_peruser/enable
curl -u admin:admin -H "Content-Type: application/json" -d '{"action":"enable_single_node"}' localhost:5984/_cluster_setup
curl -X PUT -u admin:admin -d '{"name":"bob","password":"secret","type":"user","roles":[]}' localhost:5984/_users/org.couchdb.user:bob
