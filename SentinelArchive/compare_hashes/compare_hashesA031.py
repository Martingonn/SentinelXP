#!/usr/bin/env python
# -*- coding: utf-8 -*-

import hashlib
import os
import ssdeep
import sys
import datetime

# Windows API constants for Recycle Bin (still defined if needed elsewhere)
FILE_OPERATION_DELETE = 0x0003
FOF_ALLOWUNDO = 0x0040
FOF_NOCONFIRMATION = 0x0010

LOG_FILE = "detection_log.txt"

def log_detection(message):
    """Append a timestamped message to the detection log file."""
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    try:
        with open(LOG_FILE, "a") as f:
            f.write("[{}] {}\n".format(timestamp, message))
    except Exception as e:
        print "[-] Failed to write to log file: {}".format(str(e))

def calculate_hashes(file_path):
    """Calculate both cryptographic and fuzzy hashes"""
    hashes = {
        'md5': None,
        'ssdeep': None
    }
    try:
        # MD5 Calculation
        md5 = hashlib.md5()
        with open(file_path, 'rb') as f:
            for chunk in iter(lambda: f.read(4096), b''):
                md5.update(chunk)
        hashes['md5'] = md5.hexdigest().lower()

        # SSDeep Calculation
        hashes['ssdeep'] = ssdeep.hash_from_file(file_path)
    except Exception as e:
        print "Hash calculation error: {}".format(str(e))
        log_detection("Hash calculation error for file '{}': {}".format(file_path, str(e)))
    return hashes

def load_malware_database(database_file):
    """Load malware hashes with MD5, SSDeep, and malware name"""
    malware_db = []
    try:
        with open(database_file, 'r') as f:
            for line in f:
                parts = line.strip().split(' ', 2)  # md5, ssdeep, name
                if len(parts) == 3:
                    malware_db.append({
                        'md5': parts[0].lower(),
                        'ssdeep': parts[1],
                        'name': parts[2]
                    })
    except IOError as e:
        print "Database error: {}".format(str(e))
        log_detection("Database error: {}".format(str(e)))
    return malware_db

def calculate_safety_score(file_hashes, malware_db):
    """Calculate safety score (0-100) and find most similar malware based on hashes"""
    max_similarity = 0
    most_similar_name = None

    # Check exact MD5 matches (score=0, 100% similarity)
    for entry in malware_db:
        if file_hashes['md5'] == entry['md5']:
            return 0, entry['name'], 100

    # Check fuzzy matches using ssdeep
    for entry in malware_db:
        try:
            similarity = ssdeep.compare(file_hashes['ssdeep'], entry['ssdeep'])
            if similarity > max_similarity:
                max_similarity = similarity
                most_similar_name = entry['name']
        except Exception:
            continue

    # Safety score: 100 - max_similarity (lower score = more dangerous)
    score = max(0, 100 - max_similarity)
    return score, most_similar_name, max_similarity

def delete_file_permanently(path):
    """Permanently delete a single file using os.remove()"""
    try:
        os.remove(path)
        print "[!] Permanently deleted malicious file: {}".format(path)
        log_detection("Deleted malicious file permanently: {}".format(path))
        return True
    except Exception as e:
        print "[!] Deletion failed: {}".format(str(e))
        log_detection("Failed to delete file '{}': {}".format(path, str(e)))
        return False

def quarantine_file(file_path, quarantine_dir):
    """Move file to quarantine directory specified by user"""
    try:
        if not os.path.exists(quarantine_dir):
            os.makedirs(quarantine_dir)
        new_path = os.path.join(quarantine_dir, os.path.basename(file_path))
        os.rename(file_path, new_path)
        print "[+] File quarantined at: {}".format(new_path)
        log_detection("File quarantined: {} -> {}".format(file_path, new_path))
        return True
    except Exception as e:
        print "[-] Quarantine failed: {}".format(str(e))
        log_detection("Failed to quarantine file '{}': {}".format(file_path, str(e)))
        return False

def handle_warning_actions(file_path):
    """Interactive menu for warning-level threats"""
    while True:
        print "\nSelect action:"
        print "1. Delete manually"
        print "2. Quarantine file"
        print "3. Show file hashes"
        print "4. Exit"

        choice = raw_input("Choice (1-4): ").strip()

        if choice == '1':
            confirm = raw_input("Confirm manual deletion? (y/n): ").lower()
            if confirm == 'y':
                try:
                    os.remove(file_path)
                    print "[+] File deleted manually"
                    log_detection("File manually deleted by user: {}".format(file_path))
                    return True
                except Exception as e:
                    print "[-] Deletion failed: {}".format(str(e))
                    log_detection("Manual deletion failed for file '{}': {}".format(file_path, str(e)))
            else:
                print "[*] Deletion cancelled"

        elif choice == '2':
            quarantine_dir = raw_input("Enter quarantine directory path: ").strip()
            if quarantine_file(file_path, quarantine_dir):
                return True

        elif choice == '3':
            hashes = calculate_hashes(file_path)
            print "\nMD5: {}".format(hashes['md5'])
            print "SSDeep: {}".format(hashes['ssdeep'])

        elif choice == '4':
            print "[*] Exiting action menu"
            return False

        else:
            print "[-] Invalid choice"

def main():
    print "Compare Hashes from Sentinel XP"
    file_path = raw_input("Enter file to scan: ").strip()
    if not os.path.exists(file_path):
        print "File not found"
        return

    db_path = raw_input("Enter malware database path: ").strip()
    if not os.path.exists(db_path):
        print "Database not found"
        return

    malware_db = load_malware_database(db_path)
    if not malware_db:
        print "Empty malware database"
        return

    file_hashes = calculate_hashes(file_path)
    score, malware_name, similarity = calculate_safety_score(file_hashes, malware_db)

    print "\nSecurity Analysis:"
    print "MD5: {}".format(file_hashes['md5'])
    print "SSDeep: {}".format(file_hashes['ssdeep'])
    print "Safety Score: {}/100".format(score)

    if malware_name:
        print "Most similar malware: {} (Similarity: {}%)".format(malware_name, similarity)

    if score < 16:  # Critical threshold
        print "\n[!] CRITICAL: Malicious file detected (score below 16)"
        log_detection("CRITICAL: Malicious file detected: '{}' with score {} (similar to '{}', similarity {}%)".format(
            file_path, score, malware_name, similarity))
        if delete_file_permanently(file_path):
            print "[+] File {} deleted successfully.".format(file_path)
        else:
            print "[-] File {} deletion failed.".format(file_path)
    elif 16 <= score <= 30:
        print "\n[!] WARNING: Potential malware detected [Threshold: 16-30/100]"
        print "File Safety Score: {}/100".format(score)
        print "[+] Recommended actions:"
        log_detection("WARNING: Potential malware detected: '{}' with score {} (similar to '{}', similarity {}%)".format(
            file_path, score, malware_name, similarity))
        handle_warning_actions(file_path)
    else:
        print "\n[+] File appears safe."
        log_detection("SAFE: File '{}' scanned with score {}".format(file_path, score))

if __name__ == "__main__":
    main()
