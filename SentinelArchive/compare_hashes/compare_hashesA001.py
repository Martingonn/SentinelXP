import hashlib
from similarity.simhash import Simhash

# Path to the malware SHA-sum text file
MALWARE_HASH_FILE = "malware_hashes.txt"

# Function to load known malicious hashes
def load_malware_hashes(file_path):
    try:
        with open(file_path, 'r') as f:
            return [line.strip() for line in f.readlines()]
    except FileNotFoundError:
        print(f"Error: {file_path} not found.")
        return []

# Function to calculate SHA256 hash of a file
def calculate_sha256(file_path):
    try:
        with open(file_path, 'rb') as f:
            file_content = f.read()
            return hashlib.sha256(file_content).hexdigest()
    except FileNotFoundError:
        print(f"Error: {file_path} not found.")
        return None

# Function to calculate similarity score
def calculate_similarity_score(file_hash, malware_hashes):
    max_similarity = 0
    for malware_hash in malware_hashes:
        similarity = Simhash(file_hash).similarity(Simhash(malware_hash))
        max_similarity = max(max_similarity, similarity)
    safety_score = int((1 - max_similarity) * 100)  # Higher similarity reduces safety score
    return safety_score

if __name__ == "__main__":
    # Load known malicious hashes
    malware_hashes = load_malware_hashes(MALWARE_HASH_FILE)

    # File to scan
    FILE_TO_SCAN = "example.txt"
    
    # Calculate file hash
    file_hash = calculate_sha256(FILE_TO_SCAN)
    
    if file_hash:
        # Evaluate similarity and safety score
        safety_score = calculate_similarity_score(file_hash, malware_hashes)
        print(f"Safety Score: {safety_score}/100")