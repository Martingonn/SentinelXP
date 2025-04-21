#!/usr/bin/env python
# -*- coding: utf-8 -*-

import hashlib
import os
import ssdeep
import ctypes  # Required for Recycle Bin operations
import sys

# Windows API constants for Recycle Bin
FILE_OPERATION_DELETE = 0x0003
FOF_ALLOWUNDO = 0x0040
FOF_NOCONFIRMATION = 0x0010

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
    return hashes

def load_malware_database(database_file):
    """Load malware hashes with both MD5 and SSDeep"""
    malware_db = []
    try:
        with open(database_file, 'r') as f:
            for line in f:
                parts = line.strip().split(' ', 2)
                if len(parts) >= 3:
                    malware_db.append({
                        'md5': parts[0].lower(),
                        'ssdeep': parts[1],
                        'name': parts[2]
                    })
    except IOError as e:
        print "Database error: {}".format(str(e))
    return malware_db

def calculate_safety_score(file_hashes, malware_db):
    """Calculate safety score (0-100) based on hash matches"""
    max_similarity = 0

    # Check exact MD5 matches (automatic 0 score)
    for entry in malware_db:
        if file_hashes['md5'] == entry['md5']:
            return 0

    # Check fuzzy matches
    for entry in malware_db:
        try:
            similarity = ssdeep.compare(file_hashes['ssdeep'], entry['ssdeep'])
            max_similarity = max(max_similarity, similarity)
        except:
            continue

    return max(0, 100 - max_similarity)

def delete_file_permanently(path):
    """Delete file and empty Recycle Bin (Windows XP specific)"""
    try:
        # Delete file to Recycle Bin first
        shell32 = ctypes.windll.shell32
        shell32.SHFileOperationW.argtypes = [
            ctypes.POINTER(ctypes.c_uint),
            ctypes.c_void_p
        ]

        # Convert path to Windows format
        path = os.path.abspath(path)
        path = path.replace('/', '\\')
        if not path.startswith('\\\\?\\'):
            path = '\\\\?\\' + path

        # Prepare SHFILEOPSTRUCT
        class SHFILEOPSTRUCT(ctypes.Structure):
            _fields_ = [
                ("hwnd", ctypes.c_void_p),
                ("wFunc", ctypes.c_uint),
                ("pFrom", ctypes.c_wchar_p),
                ("pTo", ctypes.c_wchar_p),
                ("fFlags", ctypes.c_ushort),
                ("fAnyOperationsAborted", ctypes.c_int),
                ("hNameMappings", ctypes.c_void_p),
                ("lpszProgressTitle", ctypes.c_wchar_p),
            ]

        params = SHFILEOPSTRUCT()
        params.wFunc = FILE_OPERATION_DELETE
        params.pFrom = path + '\0\0'  # Double null-terminated string
        params.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION

        # Perform deletion
        result = shell32.SHFileOperationW(ctypes.byref(params))
        if result != 0:
            raise RuntimeError("Deletion failed with error {}".format(result))

        # Empty Recycle Bin (Windows XP specific)
        SHEmptyRecycleBin = shell32.SHEmptyRecycleBinW
        SHEmptyRecycleBin.argtypes = [
            ctypes.c_void_p,
            ctypes.c_wchar_p,
            ctypes.c_uint
        ]
        SHEmptyRecycleBin(0, None, 0)

        print "[!] Deleted malicious file: {}".format(path)
        return True

    except Exception as e:
        print "[!] Deletion failed: {}".format(str(e))
        return False

def quarantine_file(file_path, quarantine_dir):
    """Move file to quarantine directory specified by user"""
    try:
        if not os.path.exists(quarantine_dir):
            os.makedirs(quarantine_dir)
        new_path = os.path.join(quarantine_dir, os.path.basename(file_path))
        os.rename(file_path, new_path)
        print "[+] File quarantined at: {}".format(new_path)
        return True
    except Exception as e:
        print "[-] Quarantine failed: {}".format(str(e))
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
                    return True
                except Exception as e:
                    print "[-] Deletion failed: {}".format(str(e))
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
    score = calculate_safety_score(file_hashes, malware_db)

    print "\nSecurity Analysis:"
    print "MD5: {}".format(file_hashes['md5'])
    print "SSDeep: {}".format(file_hashes['ssdeep'])
    print "Safety Score: {}/100".format(score)

    if score < 16:  # Critical threshold
        print "\n[!] CRITICAL: Malicious file detected (score below 16)"
        if delete_file_permanently(file_path):
            print "[+] File {} deleted successfully.".format(file_path)
        else:
            print "[-] File {} deletion failed.".format(file_path)
    elif 16 <= score <= 30:
        print "\n[!] WARNING: Potential malware detected [Threshold: 16-30/100]"
        print "File Safety Score: {}/100".format(score)
        print "[+] Recommended actions:"
        handle_warning_actions(file_path)
    else:
        print "\n[+] File appears safe."

if __name__ == "__main__":
    main()
