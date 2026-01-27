#!/bin/bash

FIFO_PATH="/tmp/test_fifo"

mkfifo "$FIFO_PATH" -m 666

if [[ -p "$FIFO_PATH" ]]; then
    echo "FIFO created successfully at $FIFO_PATH"
else
    echo "Failed to create FIFO at $FIFO_PATH"
    exit 1
fi
