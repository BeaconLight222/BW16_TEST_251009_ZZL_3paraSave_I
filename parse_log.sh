#!/bin/bash

# parse_log.sh
# Script to parse beacon device logs from RTF files into CSV format.
# Designed for MINGW64 / Git Bash environments.

INPUT_FILE=${1:-"v2.6.6_June24.rtf"}
OUTPUT_FILE="${INPUT_FILE}_parsed_log.csv"

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: File $INPUT_FILE not found."
    echo "Usage: ./parse_log.sh [input_file.rtf]"
    exit 1
fi

echo "Parsing $INPUT_FILE..."

# We use awk for high-performance parsing of large log files.
# The script is designed to be easily extendable by adding entries to the BEGIN block.
awk '
# Helper function to convert a hex string to a decimal value
function hex_to_dec(hex,    val, i, len, char, d) {
    val = 0
    len = length(hex)
    for (i = 1; i <= len; i++) {
        char = tolower(substr(hex, i, 1))
        d = index("0123456789abcdef", char) - 1
        if (d < 0) return 0
        val = val * 16 + d
    }
    return val
}

# Decodes object fields from the circular buffer representation
function parse_radar_object(hex_bytes, R, obj_idx, out_arr,    offset, low, high, raw_x, sign_x, abs_x, raw_y, sign_y, abs_y, raw_spd, sign_spd, abs_spd) {
    offset = (R + 4 + obj_idx * 8) % 30
    
    # x (low: offset, high: offset+1)
    low = hex_to_dec(hex_bytes[offset + 1])
    high = hex_to_dec(hex_bytes[((offset + 1) % 30) + 1])
    raw_x = high * 256 + low
    sign_x = (raw_x >= 32768) ? 1 : -1
    abs_x = raw_x % 32768
    out_arr["x"] = sign_x * abs_x
    
    # y (low: offset+2, high: offset+3)
    low = hex_to_dec(hex_bytes[((offset + 2) % 30) + 1])
    high = hex_to_dec(hex_bytes[((offset + 3) % 30) + 1])
    raw_y = high * 256 + low
    sign_y = (raw_y >= 32768) ? 1 : -1
    abs_y = raw_y % 32768
    out_arr["y"] = sign_y * abs_y
    
    # speed (low: offset+4, high: offset+5)
    low = hex_to_dec(hex_bytes[((offset + 4) % 30) + 1])
    high = hex_to_dec(hex_bytes[((offset + 5) % 30) + 1])
    raw_spd = high * 256 + low
    sign_spd = (raw_spd >= 32768) ? 1 : -1
    abs_spd = raw_spd % 32768
    out_arr["speed"] = sign_spd * abs_spd
    
    # resolution (low: offset+6, high: offset+7)
    low = hex_to_dec(hex_bytes[((offset + 6) % 30) + 1])
    high = hex_to_dec(hex_bytes[((offset + 7) % 30) + 1])
    out_arr["res"] = high * 256 + low
}

BEGIN {
    # EXTENSION POINT: Add new columns and search patterns here
    cols[1] = "time";             pat[1] = "Current RTC time: "
    cols[2] = "line_number";      pat[2] = "" 
    cols[3] = "radar1";           pat[3] = "radar1 object0 distance:"
    cols[4] = "radar2";           pat[4] = "radar2 object0 distance:"
    cols[5] = "thermal";          pat[5] = "thermal object0 distance:"
    cols[6] = "object_detected";  pat[6] = "object:"
    cols[7] = "minimal_distance"; pat[7] = "Sensor minimal distance: "
    
    # NEW UV and Exposure fields
    cols[8] = "uvLampMode";               pat[8] = "uvLampMode:"
    cols[9] = "lightState";                pat[9] = "lightState:"
    cols[10] = "scheduleEnabled";          pat[10] = "scheduleEnabled:"
    cols[11] = "lastScheduleInEffect";     pat[11] = "lastScheduleInEffect:"
    cols[12] = "accumulatedExposure";      pat[12] = "accumulatedExposure:"
    cols[13] = "accumulatedExposureThreshold"; pat[13] = "accumulatedExposureThreshold:"
    cols[14] = "calculatedMinimalDistance";          pat[14] = "calculatedMinimalDistance:"
    cols[15] = "averagedCalculatedMinimalDistance";  pat[15] = "averagedCalculatedMinimalDistance:"
    
    # Decoded Radar 0 (serial_mux=0) fields
    cols[16] = "radar0_x";                     pat[16] = ""
    cols[17] = "radar0_y";                     pat[17] = ""
    cols[18] = "radar0_speed";                 pat[18] = ""
    cols[19] = "radar0_resolution";            pat[19] = ""
    cols[20] = "radar0_data_length";           pat[20] = ""
    
    # Decoded Radar 1 (serial_mux=1) fields
    cols[21] = "radar1_x";                     pat[21] = ""
    cols[22] = "radar1_y";                     pat[22] = ""
    cols[23] = "radar1_speed";                 pat[23] = ""
    cols[24] = "radar1_resolution";            pat[24] = ""
    cols[25] = "radar1_data_length";           pat[25] = ""
    
    # Logged Object 0 fields for Radar 1 and Radar 2
    cols[26] = "radar1_log_x";                 pat[26] = ""
    cols[27] = "radar1_log_y";                 pat[27] = ""
    cols[28] = "radar2_log_x";                 pat[28] = ""
    cols[29] = "radar2_log_y";                 pat[29] = ""
    
    num_cols = 29

    # uvLampMode Mapping
    uvMap[0]   = "MODE_SMART"
    uvMap[1]   = "MODE_MANUAL"
    uvMap[2]   = "MODE_UNLIMITED"
    uvMap[3]   = "MODE_OCCUPIED_ROOM"
    uvMap[4]   = "MODE_EMPTY_ROOM"
    uvMap[252] = "MODE_IDLE_OFF"    # 0xFC
    uvMap[253] = "MODE_IDLE_MANUAL" # 0xFD
    uvMap[255] = "MODE_OFF"         # 0xFF

    # Print CSV Header
    for (i = 1; i <= num_cols; i++) {
        printf "%s%s", cols[i], (i == num_cols ? ORS : ",")
    }
}

# Start of a new execution loop
/Beacon Device Loop Start/ {
    in_loop = 1
    # Clear previous values
    for (i = 1; i <= num_cols; i++) val[i] = ""
    # Capture the line number of the start of the loop
    val[2] = NR
    
    # Reset parsing state variables
    pending_hex_for = ""
    pending_hex_index = ""
    pending_hex_len = ""
    pending_data_for = ""
}

# End of the current execution loop - output the collected data
/Beacon Device Loop END/ {
    if (in_loop) {
        for (i = 1; i <= num_cols; i++) {
            # Trim potential trailing carriage return or spaces
            gsub(/\r/, "", val[i])
            sub(/[ \t]+$/, "", val[i])
            printf "%s%s", val[i], (i == num_cols ? ORS : ",")
        }
        in_loop = 0
    }
}

# Extract data if we are inside a loop
in_loop {
    # Fix "mashed" fields by inserting a space before the next field name
    gsub(/accumulatedExposureThreshold:/, " accumulatedExposureThreshold:")
    gsub(/averagedCalculatedMinimalDistance:/, " averagedCalculatedMinimalDistance:")
    
    # Basic RTF cleanup
    gsub(/\\[a-z0-9]* ?/, "")

    # If we are expecting the HEX data dump line
    if (pending_hex_for != "") {
        n_bytes = split($0, hex_bytes, " ")
        if (n_bytes >= 30) {
            delete out_obj
            parse_radar_object(hex_bytes, pending_hex_index, 0, out_obj)
            if (pending_hex_for == "0") {
                val[16] = out_obj["x"]
                val[17] = out_obj["y"]
                val[18] = out_obj["speed"]
                val[19] = out_obj["res"]
                val[20] = pending_hex_len
            } else if (pending_hex_for == "1") {
                val[21] = out_obj["x"]
                val[22] = out_obj["y"]
                val[23] = out_obj["speed"]
                val[24] = out_obj["res"]
                val[25] = pending_hex_len
            }
        }
        pending_hex_for = ""
    }

    # Match and parse Radar Dump header line
    if ($0 ~ /Radar Dump:/) {
        p_dump = index($0, "Radar Dump: ")
        p_cnt  = index($0, "radarObjectCount: ")
        p_idx  = index($0, "Index: ")
        p_len  = index($0, "Data Length: ")
        
        if (p_dump > 0 && p_cnt > 0 && p_idx > 0 && p_len > 0) {
            s_dump = substr($0, p_dump + 12)
            split(s_dump, parts_dump, /[ \t,\r]/)
            dump_id = parts_dump[1]
            
            s_idx = substr($0, p_idx + 7)
            split(s_idx, parts_idx, /[ \t,\r]/)
            dump_index = parts_idx[1]
            
            s_len = substr($0, p_len + 13)
            split(s_len, parts_len, /[ \t,\r]/)
            data_len = parts_len[1]
            
            pending_hex_for = dump_id
            pending_hex_index = dump_index
            pending_hex_len = data_len
        }
    }

    # Match Radar data headers
    if ($0 ~ /Radar1 data:/) {
        pending_data_for = 1
    } else if ($0 ~ /Radar2 data:/) {
        pending_data_for = 2
    }

    # Match Object 0 line for Radar 1/2 logs
    if ($0 ~ /Object 0:/) {
        if (pending_data_for == 1) {
            p_obj = index($0, "Object 0: ")
            if (p_obj > 0) {
                s_obj = substr($0, p_obj + 10)
                n_parts = split(s_obj, parts_obj, /[ \t,\r]+/)
                if (n_parts >= 2) {
                    val[26] = parts_obj[1]
                    val[27] = parts_obj[2]
                }
            }
            pending_data_for = 0
        } else if (pending_data_for == 2) {
            p_obj = index($0, "Object 0: ")
            if (p_obj > 0) {
                s_obj = substr($0, p_obj + 10)
                n_parts = split(s_obj, parts_obj, /[ \t,\r]+/)
                if (n_parts >= 2) {
                    val[28] = parts_obj[1]
                    val[29] = parts_obj[2]
                }
            }
            pending_data_for = 0
        }
    }

    # Original pattern matches
    for (i = 1; i <= num_cols; i++) {
        if (pat[i] == "") continue

        # Specific fix for "object:" to avoid partial matches
        if (i == 6 && $0 !~ /^object:/) continue

        p_idx = index($0, pat[i])
        if (p_idx > 0) {
            s = substr($0, p_idx + length(pat[i]))
            sub(/^[ \t]+/, "", s)
            split(s, parts, /[ \t,]/)
            extracted = parts[1]
            
            # Map uvLampMode (index 8) to its enum label
            if (i == 8) {
                if (extracted in uvMap) {
                    extracted = uvMap[extracted]
                } else if (extracted != "") {
                    extracted = "UNKNOWN"
                }
            }
            
            val[i] = extracted
        }
    }
}
' "$INPUT_FILE" > "$OUTPUT_FILE"

echo "Done! Data extracted to $OUTPUT_FILE"
# Count total lines minus header
count=$(wc -l < "$OUTPUT_FILE")
echo "Total loops processed: $((count - 1))"
