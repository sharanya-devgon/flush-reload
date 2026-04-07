#!/bin/bash

while true; do
  gpg --batch --yes --clearsign secret.txt
done