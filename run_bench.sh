#!/usr/bin/env bash

#usage: ./run_bench.sh <num_clients>

if [ $# -lt 1 ]; then
    echo "Usage: $0 <num_clients>"
    exit 1
fi

NUM_CLIENTS="$1"
SERVER_LOG="server_output.log"

#1. run server in background, output to file
build/run_streamer data/CLX5_mbo.dbn > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!

#small delay to ensure server starts
sleep 1

#2. run client with NUM_CLIENTS as argument
build/run_builder "$NUM_CLIENTS"

#3. wait for server to finish (optional, safe)
wait "$SERVER_PID" 2>/dev/null

#4. print server output
cat "$SERVER_LOG"

#5. remove server log
rm -f "$SERVER_LOG"

