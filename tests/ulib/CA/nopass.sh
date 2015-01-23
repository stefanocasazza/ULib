#!/bin/sh

openssl rsa -in server.key -passin pass:caciucco -out server_nopass.key
