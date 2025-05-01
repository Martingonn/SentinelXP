import os
import struct
from datetime import datetime

def create_formatted_image(filename, label):
    """Create pre-formatted FAT12 image with directory structure"""
    # Boot sector + FATs + Root Directory + Data Area
    boot_sector = (
        b'\xEB\x3C\x90' +          # Jump instruction
        b'LABEL   ' +              # OEM Name (8 chars)
        b'\x00\x02' +              # Bytes per sector (512)
        b'\x01' +                  # Sectors per cluster
        b'\x01\x00' +              # Reserved sectors
        b'\x02' +                  # FAT copies
        b'\xE0\x00' +              # Root entries (224)
        b'\x40\x0B' +              # Total sectors (2880)
        b'\xF0' +                  # Media descriptor
        b'\x09\x00' +              # Sectors per FAT
        b'\x12\x00' +              # Sectors per track
        b'\x02' +                  # Number of heads
        b'\x00\x00\x00\x00' +      # Hidden sectors
        b'\x00\x00\x00\x00' +      # Total sectors large
        b'\x00' +                  # Drive number
        b'\x00' +                  # Reserved
        b'\x29' +                  # Extended boot signature
        b'\x00\x00\x00' +          # Volume serial number
        label.ljust(11).encode() + # Volume label
        b'FAT12   ' +              # File system type
        b'\x00' * 448 +            # Boot code
        b'\x55\xAA'                # Signature
    )
    
    # FAT Tables (2 copies)
    fat = b'\xF0\xFF\xFF' + b'\x00' * 4605  # 9 sectors * 512 = 4608 bytes
    
    # Root Directory (14 sectors)
    root_dir = b'\x00' * 14 * 512
    
    # Data Area (2847 sectors)
    data = b'\x00' * 2847 * 512
    
    with open(filename, 'wb') as f:
        f.write(boot_sector)
        f.write(fat)
        f.write(fat)  # Second FAT copy
        f.write(root_dir)
        f.write(data)

def add_file_to_image(image_path, file_path):
    """Add file to FAT12 image with proper directory entry and cluster allocation"""
    with open(image_path, 'r+b') as img:
        # Read existing FAT
        img.seek(512)  # FAT1 starts at sector 1
        fat = img.read(9 * 512)  # Read first FAT
        
        # Find first free cluster
        cluster = 2  # First data cluster
        while cluster < len(fat)//1.5:  # FAT12 uses 1.5 bytes per entry
            offset = cluster * 3 // 2
            if cluster % 2 == 0:
                value = struct.unpack('<H', fat[offset:offset+2])[0] & 0x0FFF
            else:
                value = struct.unpack('<H', fat[offset:offset+2])[0] >> 4
            if value == 0:
                break
            cluster += 1

        if cluster >= 4085:  # Max clusters for FAT12
            raise Exception("No free clusters available")

        # Read file content
        with open(file_path, 'rb') as f:
            content = f.read()
        
        # Calculate needed clusters (1 cluster = 512 bytes)
        clusters_needed = len(content) // 512 + 1
        
        # Update FAT
        for i in range(clusters_needed):
            next_cluster = cluster + 1 if i < clusters_needed - 1 else 0xFFF
            offset = cluster * 3 // 2
            if cluster % 2 == 0:
                new_value = (fat[offset+1] & 0xF0) | (next_cluster & 0x0F)
                new_value |= ((next_cluster & 0x0FF0) << 4)
                fat = fat[:offset] + struct.pack('<H', new_value)[:2] + fat[offset+2:]
            else:
                new_value = (next_cluster << 4) | (fat[offset] & 0x0F)
                fat = fat[:offset] + struct.pack('<H', new_value)[:2] + fat[offset+2:]
            cluster = next_cluster

        # Write updated FAT
        img.seek(512)
        img.write(fat)
        img.seek(512 + 9*512)  # Write to FAT2
        img.write(fat)

        # Create directory entry
        name = os.path.basename(file_path).upper()[:8]
        ext = os.path.splitext(file_path)[1][1:4].upper()
        dir_entry = (
            name.ljust(8).encode('ascii') +
            ext.ljust(3).encode('ascii') +
            b'\x20' +  # Attributes (archive)
            b'\x00' +  # Case/Reserved
            b'\x00' +  # Creation time (ms)
            struct.pack('<H', 0x0000) +  # Creation time
            struct.pack('<H', 0x0000) +  # Creation date
            struct.pack('<H', 0x0000) +  # Last access date
            struct.pack('<H', 0x0000) +  # High cluster
            struct.pack('<H', 0x0000) +  # Modification time
            struct.pack('<H', 0x0000) +  # Modification date
            struct.pack('<H', cluster) +  # Start cluster
            struct.pack('<I', len(content))  # File size
        )

        # Find free directory entry
        img.seek(19*512)  # Root directory start
        root_dir = img.read(14*512)
        for i in range(0, len(root_dir), 32):
            if root_dir[i] == 0x00 or root_dir[i] == 0xE5:
                img.seek(19*512 + i)
                img.write(dir_entry)
                break

        # Write file content
        img.seek(33*512 + (cluster-2)*512)  # Data area start + cluster offset
        img.write(content)
        img.write(b'\x00' * (512 - len(content) % 512))  # Padding

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
        if os.path.exists(file_path):
            try:
                add_file_to_image(image_path, file_path)
                print(f"Added {os.path.basename(file_path)}")
            except Exception as e:
                print(f"Error adding file: {str(e)}")
        else:
            print("File not found!")

if __name__ == "__main__":
    main()
