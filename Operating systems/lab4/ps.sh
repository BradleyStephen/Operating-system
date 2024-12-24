#!/bin/bash

# ELEC377 - Operating System
# Lab 4 - Shell Scripting, ps.sh
# Program Description: Lists running processes and displays specified information based on flags.

showRSS="no"
showCommand="no"
showComm="no"
showGroup="no"

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -rss) showRSS="yes";;
        -command) 
            if [[ "$showComm" == "yes" ]]; then
                echo "Error: Cannot use -comm and -command together."
                exit 1
            fi
            showCommand="yes";;
        -comm) 
            if [[ "$showCommand" == "yes" ]]; then
                echo "Error: Cannot use -comm and -command together."
                exit 1
            fi
            showComm="yes";;
        -group) showGroup="yes";;
        *) echo "Error: Invalid flag $1"; exit 1;;
    esac
    shift
done

# Prepare temporary file with unique name
tmp_file="/tmp/tmpPs$$.txt"
> "$tmp_file"  # Create file or clear if it exists

# Main loop to gather process information
for p in /proc/[0-9]*; do
    if [[ -d "$p" ]]; then
        # Retrieve process information using awk directly
        pid=$(awk '/^Pid:/ {print $2}' "$p/status")
        user_id=$(awk '/^Uid:/ {print $2}' "$p/status")
        group_id=$(awk '/^Gid:/ {print $2}' "$p/status")

        # Retrieve RSS and set to 0 if empty
        rss=$(awk '/^VmRSS:/ {print $2}' "$p/status")
        rss=${rss:-0}  # Ensures RSS is set to 0 if not available

        # Convert numeric user and group IDs to names
        user_name=$(getent passwd "$user_id" | cut -d ':' -f1)
        group_name=$(getent group "$group_id" | cut -d ':' -f1)

        # Skip processes if user_name or group_name is empty
        if [[ -z "$user_name" || -z "$group_name" ]]; then
            continue
        fi

        # Retrieve command based on specified flags
        if [[ "$showCommand" == "yes" ]]; then
            command=$(tr '\0' ' ' < "$p/cmdline" | cut -c 1-50)  # Limits command to first 50 characters
            [[ -z "$command" ]] && command=$(awk '/^Name:/ {print $2}' "$p/status")
        elif [[ "$showComm" == "yes" ]]; then
            command=$(awk '/^Name:/ {print $2}' "$p/status")
        else
            command=""
        fi

        # Adjust output line format based on flags
        if [[ "$showRSS" == "yes" && "$showGroup" == "yes" && ( "$showCommand" == "yes" || "$showComm" == "yes" ) ]]; then
            printf "%-10s %-15s %-15s %-10s %-50s\n" "$pid" "$user_name" "$group_name" "$rss" "$command" >> "$tmp_file"
        elif [[ "$showRSS" == "yes" && "$showGroup" == "yes" ]]; then
            printf "%-10s %-15s %-15s %-10s\n" "$pid" "$user_name" "$group_name" "$rss" >> "$tmp_file"
        elif [[ "$showRSS" == "yes" ]]; then
            printf "%-10s %-15s %-10s\n" "$pid" "$user_name" "$rss" >> "$tmp_file"
        elif [[ "$showGroup" == "yes" ]]; then
            printf "%-10s %-15s %-15s\n" "$pid" "$user_name" "$group_name" >> "$tmp_file"
        else
            printf "%-10s %-15s\n" "$pid" "$user_name" >> "$tmp_file"
        fi
    fi
done

# Print header based on flags with aligned columns
if [[ "$showRSS" == "yes" && "$showGroup" == "yes" && ( "$showCommand" == "yes" || "$showComm" == "yes" ) ]]; then
    printf "%-10s %-15s %-15s %-10s %-50s\n" "PID" "USER" "GROUP" "RSS" "COMMAND"
elif [[ "$showRSS" == "yes" && "$showGroup" == "yes" ]]; then
    printf "%-10s %-15s %-15s %-10s\n" "PID" "USER" "GROUP" "RSS"
elif [[ "$showRSS" == "yes" ]]; then
    printf "%-10s %-15s %-10s\n" "PID" "USER" "RSS"
elif [[ "$showGroup" == "yes" ]]; then
    printf "%-10s %-15s %-15s\n" "PID" "USER" "GROUP"
else
    printf "%-10s %-15s\n" "PID" "USER"
fi

# Print sorted output with aligned columns
sort -n "$tmp_file" | column -t

# Remove the temporary file if it exists
if [[ -f "$tmp_file" ]]; then
    rm "$tmp_file"
fi
