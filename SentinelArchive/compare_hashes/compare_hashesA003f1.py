import hashlib
from similarity.simhash import Simhash
import os
import shutil
from datetime import datetime

# Path to the malware SHA-sum and name text file
MALWARE_HASH_FILE = str(input("Input path to malware hashes: "))
LOG_FILE = str(input("Input log file path: "))

# Function to load known malicious hashes and names
def load_malware_info(file_path):
    try:
        malware_info = {}
        with open(file_path, 'r') as f:
            for line in f.readlines():
                # Assuming format: "hash malware_name"
                parts = line.strip().split()
                if len(parts) >= 2:
                    malware_info[parts[0]] = ' '.join(parts[1:])
                else:
                    print(f"Invalid format in {file_path}: {line}")
        return malware_info
    except FileNotFoundError:
        print(f"Error: {file_path} not found.")
        return {}

# Function to calculate SHA256 hash of a file
def calculate_sha256(file_path):
    try:
        with open(file_path, 'rb') as f:
            file_content = f.read()
            return hashlib.sha256(file_content).hexdigest()
    except FileNotFoundError:
        print(f"Error: {file_path} not found.")
        return None

# Function to calculate similarity score and find most similar malware
def calculate_similarity_score(file_hash, malware_info):
    max_similarity = 0
    most_similar_malware = None
    for malware_hash, malware_name in malware_info.items():
        similarity = Simhash(file_hash).similarity(Simhash(malware_hash))
        if similarity > max_similarity:
            max_similarity = similarity
            most_similar_malware = malware_name
    safety_score = int((1 - max_similarity) * 100)  # Higher similarity reduces safety score
    return safety_score, most_similar_malware

# Function to permanently delete a file (including clearing from Recycle Bin)
def delete_file_permanently(file_path):
    try:
        if os.path.exists(file_path):
            os.remove(file_path)
            print(f"File '{file_path}' deleted permanently.")
            
            # Clear Recycle Bin (Windows XP equivalent or modern systems)
            recycle_bin_path = os.path.join(os.environ["SystemDrive"], "$Recycle.Bin")
            if os.path.exists(recycle_bin_path):
                shutil.rmtree(recycle_bin_path)
                print("Recycle Bin cleared.")
        else:
            print(f"File '{file_path}' not found.")
    except PermissionError:
        print(f"Permission denied: Unable to delete '{file_path}'.")

# Function to log events to a log file
def log_event(event_type, file_name, malware_name=None, safety_score=None):
    with open(LOG_FILE, 'a') as log_file:
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        if event_type == "DELETED":
            log_file.write(f"{timestamp} - DELETED: '{file_name}' (Malware: {malware_name})\n")
        elif event_type == "DETECTED":
            log_file.write(f"{timestamp} - DETECTED: '{file_name}' (Malware: {malware_name}, Safety Score: {safety_score})\n")

if __name__ == "__main__":
    # Load known malicious hashes and names
    malware_info = load_malware_info(MALWARE_HASH_FILE)

    # File to scan
    FILE_TO_SCAN = str(input("Input file to scan: "))
    
    # Calculate file hash
    file_hash = calculate_sha256(FILE_TO_SCAN)
    
    if file_hash:
        # Evaluate similarity and safety score
        safety_score, most_similar_malware = calculate_similarity_score(file_hash, malware_info)
        
        if safety_score < 5:
            print(f"Very dangerous! File '{FILE_TO_SCAN}' matches closely with {most_similar_malware}. Deleting automatically...")
            delete_file_permanently(FILE_TO_SCAN)
            log_event("DELETED", FILE_TO_SCAN, most_similar_malware)
        elif safety_score < 15:
            print(f"Very likely malware: File '{FILE_TO_SCAN}' is similar to {most_similar_malware}.")
            user_response = input("Do you want to delete this file? (yes/no): ").strip().lower()
            if user_response == "yes":
                delete_file_permanently(FILE_TO_SCAN)
                log_event("DELETED", FILE_TO_SCAN, most_similar_malware)
            else:
                log_event("DETECTED", FILE_TO_SCAN, most_similar_malware, safety_score)
        elif safety_score < 30:
            print(f"Likely malware: File '{FILE_TO_SCAN}' is similar to {most_similar_malware}.")
            user_response = input("Do you want to delete this file? (yes/no): ").strip().lower()
            if user_response == "yes":
                delete_file_permanently(FILE_TO_SCAN)
                log_event("DELETED", FILE_TO_SCAN, most_similar_malware)
            else:
                log_event("DETECTED", FILE_TO_SCAN, most_similar_malware, safety_score)
        else:
            print(f"Safety Score: {safety_score}/100. File '{FILE_TO_SCAN}' appears clean.")
