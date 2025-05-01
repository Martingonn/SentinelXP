import os
import struct

SECTOR_SIZE = 512
FAT_SECTORS = 9
NUM_FATS = 2
ROOT_DIR_ENTRIES = 224
ROOT_DIR_SECTORS = (ROOT_DIR_ENTRIES * 32) // SECTOR_SIZE  # 14 sectors
TOTAL_SECTORS = 2880  # 1.44MB floppy
DATA_START_SECTOR = 1 + FAT_SECTORS * NUM_FATS + ROOT_DIR_SECTORS
CLUSTER_SIZE = SECTOR_SIZE  # 1 sector per cluster
MAX_CLUSTER = (TOTAL_SECTORS - DATA_START_SECTOR)  # Number of clusters

def create_boot_sector(label):
    bs = bytearray(SECTOR_SIZE)
    bs[0:3] = b'\xEB\x3C\x90'  # JMP instruction + NOP
    bs[3:11] = b'MSDOS5.0'     # OEM Name (8 bytes)
    bs[11:13] = SECTOR_SIZE.to_bytes(2, 'little')  # Bytes per sector
    bs[13] = 1                 # Sectors per cluster
    bs[14:16] = (1).to_bytes(2, 'little')    # Reserved sectors
    bs[16] = NUM_FATS          # Number of FATs
    bs[17:19] = ROOT_DIR_ENTRIES.to_bytes(2, 'little')  # Max root dir entries
    bs[19:21] = TOTAL_SECTORS.to_bytes(2, 'little')     # Total sectors (small)
    bs[21] = 0xF0              # Media descriptor
    bs[22:24] = FAT_SECTORS.to_bytes(2, 'little')       # Sectors per FAT
    bs[24:26] = (18).to_bytes(2, 'little')              # Sectors per track
    bs[26:28] = (2).to_bytes(2, 'little')               # Number of heads
    bs[28:32] = (0).to_bytes(4, 'little')               # Hidden sectors
    bs[32:36] = (0).to_bytes(4, 'little')               # Total sectors (large)
    bs[36] = 0                 # Drive number
    bs[37] = 0                 # Reserved
    bs[38] = 0x29              # Extended boot signature
    bs[39:43] = os.urandom(4)  # Volume serial number (random)
    bs[43:54] = label.ljust(11).encode('ascii')  # Volume label
    bs[54:62] = b'FAT12   '    # File system type
    bs[510:512] = b'\x55\xAA'  # Boot sector signature
    return bs

def create_fat():
    fat = bytearray(FAT_SECTORS * SECTOR_SIZE)
    # FAT12 reserved entries:
    # Entry 0: media descriptor + reserved bits
    # Entry 1: reserved, set to 0xFFF (end of cluster chain)
    fat[0] = 0xF0
    fat[1] = 0xFF
    fat[2] = 0xFF
    return fat

def get_fat_entry(fat, cluster):
    offset = (cluster * 3) // 2
    if cluster % 2 == 0:
        return fat[offset] | ((fat[offset + 1] & 0x0F) << 8)
    else:
        return ((fat[offset] >> 4) | (fat[offset + 1] << 4)) & 0xFFF

def set_fat_entry(fat, cluster, value):
    offset = (cluster * 3) // 2
    if cluster % 2 == 0:
        fat[offset] = value & 0xFF
        fat[offset + 1] = (fat[offset + 1] & 0xF0) | ((value >> 8) & 0x0F)
    else:
        fat[offset] = (fat[offset] & 0x0F) | ((value << 4) & 0xF0)
        fat[offset + 1] = (value >> 4) & 0xFF

def find_free_clusters(fat, count):
    free_clusters = []
    for cluster in range(2, MAX_CLUSTER + 2):
        if get_fat_entry(fat, cluster) == 0:
            free_clusters.append(cluster)
            if len(free_clusters) == count:
                return free_clusters
    return None

def cluster_to_sector(cluster):
    return DATA_START_SECTOR + (cluster - 2)

def create_root_dir_entry(filename, start_cluster, filesize):
    # Split filename into 8.3 format
    if '.' in filename:
        name, ext = filename.split('.', 1)
    else:
        name, ext = filename, ''
    name = name.upper()[:8].ljust(8)
    ext = ext.upper()[:3].ljust(3)
    entry = bytearray(32)
    entry[0:8] = name.encode('ascii')
    entry[8:11] = ext.encode('ascii')
    entry[11] = 0x20  # Archive attribute
    # Zero timestamps for simplicity
    entry[12:22] = b'\x00' * 10
    entry[26:28] = struct.pack('<H', start_cluster)
    entry[28:32] = struct.pack('<I', filesize)
    return entry

def find_free_root_dir_entry(root_dir):
    for i in range(0, len(root_dir), 32):
        if root_dir[i] == 0x00 or root_dir[i] == 0xE5:
            return i
    return None

def create_formatted_image(filename, label):
    with open(filename, 'wb') as f:
        f.write(create_boot_sector(label))
        fat = create_fat()
        f.write(fat)
        f.write(fat)  # FAT copy 2
        f.write(b'\x00' * ROOT_DIR_SECTORS * SECTOR_SIZE)
        data_sectors = TOTAL_SECTORS - (1 + NUM_FATS * FAT_SECTORS + ROOT_DIR_SECTORS)
        f.write(b'\x00' * data_sectors * SECTOR_SIZE)

def add_file_to_image(image_path, file_path):
    with open(image_path, 'r+b') as img:
        # Read FAT1
        img.seek(SECTOR_SIZE)
        fat = bytearray(img.read(FAT_SECTORS * SECTOR_SIZE))

        # Read root directory
        root_dir_offset = SECTOR_SIZE * (1 + NUM_FATS * FAT_SECTORS)
        img.seek(root_dir_offset)
        root_dir = bytearray(img.read(ROOT_DIR_SECTORS * SECTOR_SIZE))

        # Read file content
        with open(file_path, 'rb') as f:
            content = f.read()
        filesize = len(content)
        clusters_needed = (filesize + CLUSTER_SIZE - 1) // CLUSTER_SIZE
        if clusters_needed == 0:
            clusters_needed = 1

        free_clusters = find_free_clusters(fat, clusters_needed)
        if free_clusters is None:
            raise Exception("No free clusters available")

        # Chain clusters in FAT
        for i in range(clusters_needed):
            if i == clusters_needed - 1:
                set_fat_entry(fat, free_clusters[i], 0xFFF)  # EOF
            else:
                set_fat_entry(fat, free_clusters[i], free_clusters[i + 1])

        # Write FAT copies
        img.seek(SECTOR_SIZE)
        img.write(fat)
        img.seek(SECTOR_SIZE + FAT_SECTORS * SECTOR_SIZE)
        img.write(fat)

        # Find free root directory entry
        free_dir_index = find_free_root_dir_entry(root_dir)
        if free_dir_index is None:
            raise Exception("No free root directory entries")

        filename = os.path.basename(file_path)
        dir_entry = create_root_dir_entry(filename, free_clusters[0], filesize)

        img.seek(root_dir_offset + free_dir_index)
        img.write(dir_entry)

        # Write file data cluster by cluster
        for i, cluster in enumerate(free_clusters):
            sector = cluster_to_sector(cluster)
            img.seek(sector * SECTOR_SIZE)
            start = i * CLUSTER_SIZE
            end = start + CLUSTER_SIZE
            chunk = content[start:end]
            if len(chunk) < CLUSTER_SIZE:
                chunk += b'\x00' * (CLUSTER_SIZE - len(chunk))
            img.write(chunk)

def main():
    label = input("Input floppy label (max 11 chars): ")[:11]
    name = input("Input image name (without .img): ")
    image_path = f"{name}.img"

    create_formatted_image(image_path, label)
    print(f"Created {image_path}")

    while True:
        file_path = input("Enter file path to add (empty to finish): ").strip()
        if not file_path:
            break
        if os.path.isfile(file_path):
            try:
                add_file_to_image(image_path, file_path)
                print(f"Added {os.path.basename(file_path)}")
            except Exception as e:
                print(f"Error adding file: {e}")
        else:
            print("File not found!")

if __name__ == "__main__":
    main()
