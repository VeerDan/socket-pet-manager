#!/bin/bash

if [[ -d "./build_client" ]]; then
    rm -r "./build_client"
fi

if [[ -d "./build_server" ]]; then
    rm -r "./build_server"
fi

if [[ -f "./client.exe" ]]; then
    rm "./client.exe"
fi

if [[ -f "./server.exe" ]]; then
    rm "./server.exe"
fi
