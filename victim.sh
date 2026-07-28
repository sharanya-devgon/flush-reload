#!/bin/bash

taskset -c 1 gpg --batch --yes -u 67D56B6E --clearsign secret.txt
