#!/bin/bash

while true; do
  gpg --batch --yes --clearsign secret.txt
  sleep 0.1
done