# Script to analyze file system summary (csv). The script performs
# three scans of the file system. WHile this isn't the most performant
# implementation, it is sufficient given the circumstances.

import sys 


is_consistent = True;
reserved_blocks = set([0, 1, 2, 3, 4, 5, 6, 7, 64])
reserved_inodes = set([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])








# These section defines the fundamental data-structures used during the
# audit of the file system. These objects do not necessarily contain all
# csv fields, and may need to be revised as necessary
########################################################################

class superblk:
    def __init__(self, entry):
        self.blk_cnt   = int(entry[1])
        self.ino_cnt   = int(entry[2])
        self.blk_sz    = int(entry[3])
        self.ino_sz    = int(entry[4])
        self.blk_p_gp  = int(entry[5])
        self.ino_p_gp  = int(entry[6])
        self.first_ino = int(entry[7])

class inode:
    def __init__(self, entry):
        self.ino_num    = int(entry[1])
        self.file_type  = entry[2]
        self.link_cnt   = int(entry[6])
        self.blk_cnt    = int(entry[11])
        self.blk_addrs  = [int(i) for i in entry[12:27]]
########################################################################


inode_dict   = {}  # list of inode objects


block_dict   = {}  # hashmap, key = blk_num

# set of free blocks
bfree = set()

# set of free inodes
ifree = set()

# inode-link_count mapping
# key: inode number
# value: link count
inode_dict_lnk_cnt = {}

# key: inode number
# value: ref link count
inode_dict_ref_lnk_cnt = {}

# key: inode number
# value: parent inode
inode_dict_lr = {}

# key: parent inode number
# value: inode number
# stores only '..' directories
inode_dict_parents = {}

# referenced inodes
# key: inode number
# value: diredctory name
ref_inode = {}


# total arguments
if len(sys.argv) != 2:
    print("Incorrect number of arguments:", n)
    exit(1)


# file name
csv_file = sys.argv[1]
print("Name of csv_file:", csv_file)


try:
    csv_file = open(sys.argv[1], "r")
except:
    print('file does not exist\n')
    exit(1)

# get all the lines the file to store locally
csv_lines = csv_file.readlines()




for line in csv_lines:
    entry = line.split(",")
    summary_type = entry[0]
    
    if summary_type == 'SUPERBLOCK':
        superblock = superblk(entry)
    
    # put free inodes in free inode set
    elif summary_type == 'IFREE':
        ifree.add(int(entry[1]))
    
    # put free blocks in free block set
    elif summary_type == 'BFREE':
        bfree.add(int(entry[1]))
    
    # construct new inode object, add to dictionary, check blocks
    elif summary_type == 'INODE':
        inode_num = int(entry[1])
        cur_inode = inode(entry)
        inode_dict[inode_num] = cur_inode
        inode_dict_lnk_cnt[inode_num] = cur_inode.link_cnt
        
        for i in range(0,15):
            block_num = cur_inode.blk_addrs[i]
            if block_num == 0: # unused block address
                continue
                
            if i < 12:
                strlvl = ""
                offset = 0
                level = 0
            elif i == 12:
                strlvl = " INDIRECT"
                offset = 12
                level = 1
            elif i == 13:
                strlvl = " DOUBLE INDIRECT"
                offset = 268
                level = 2
            elif i == 14:
                strlvl = " TRIPLE INDIRECT"
                offset = 65804
                level = 3
            
            # Check for invalid blocks
            if block_num < 0 or block_num > superblock.blk_cnt:
                is_consistent = False
                print('INVALID' + strlvl + ' BLOCK '
                       + str(block_num) + ' IN INODE '
                       + str(inode_num) + ' AT OFFSET '
                       + str(offset))
                    
            # Check for allocated reserved blocks
            if block_num != 0 and block_num in reserved_blocks:
                is_consistent = False
                print('RESERVED' + strlvl + ' BLOCK '
                        + str(block_num) + ' IN INODE '
                        + str(inode_num) + ' AT OFFSET '
                        + str(offset) )
            
            # Save information about references
            elif block_num not in block_dict:
                block_dict[block_num] = [[inode_num, offset, level]]
            else:
                block_dict[block_num].append([inode_num, offset, level])
            
    elif summary_type == 'INDIRECT':
        block_num = int(entry[5])
        inode_num = int(entry[1])
        
        level = int(entry[2])
        if level == 1:
            strlvl = "INDIRECT"
            offset = 12
        elif level == 2:
            strlvl = "DOUBLE INDIRECT"
            offset = 268
        elif level == 3:
            strlvl = "TRIPLE INDIRECT"
            offset = 65804
        
        # Check for invalid blocks
        if block_num < 0 or block_num > superblock.blk_cnt:
            is_consistent = False
            print('INVALID' + strlvl + ' BLOCK '
                    + str(block_num) + ' IN INODE '
                    + str(inode_num) + ' AT OFFSET '
                    + str(offset))
                
        # Check for allocated reserved blocks
        if block_num != 0 and block_num in reserved_blocks:
            is_consistent = False
            print('RESERVED' + strlvl + ' BLOCK '
                   + str(block_num) + ' IN INODE '
                   + str(inode_num) + ' AT OFFSET '
                   + str(offset))
        
        # Save information about references
        elif block_num not in block_dict:
            block_dict[block_num] = [[inode_num, offset, level]]
        else:
            block_dict[block_num].append([inode_num, offset, level])
            
            
    elif summary_type == 'DIRENT': # put in inode dict (link ref)
        dir_name = entry[6]
        inode_num = int(entry[3])
        parent_inode = int(entry[1])
            
        ref_inode[inode_num] = dir_name
        
        if inode_num not in inode_dict_ref_lnk_cnt:
            inode_dict_ref_lnk_cnt[inode_num] = 1
        else:
            inode_dict_ref_lnk_cnt[inode_num] += 1
        
        
        # check validity, inodes begin at 1
        if inode_num < 1 or inode_num > superblock.ino_cnt:
            is_consistent = False
            print('DIRECTORY INODE '
                    + str(parent_inode) + ' NAME '
                    + dir_name[0:len(dir_name)- 2]
                    + ' INVALID INODE ' + str(inode_num))
            continue
        
        if dir_name[0:3] == "'.'" and parent_inode != inode_num:
            is_consistent = False
            print('DIRECTORY INODE ' + str(parent_inode)
                    + " NAME '.' LINK TO INODE " + str(inode_num)
                    + ' SHOULD BE ' + str(parent_inode) )
            
        elif dir_name[0:3] == "'.'":
            continue

        elif dir_name[0:4] == "'..'":
            inode_dict_parents[parent_inode] = inode_num

        else:
            inode_dict_lr[inode_num] = parent_inode
        
# We have now populated all data structures with information from
# the csv file. At this point, we are performing an audit on the
# information read in, and we processing the 'raw' csv lines.


# links vs link count
for ino_num in inode_dict_lnk_cnt:
    lnk_cnt = inode_dict_lnk_cnt[ino_num]

    if ino_num in inode_dict_ref_lnk_cnt:
        links = inode_dict_ref_lnk_cnt[ino_num]
    else:
        links = 0

    if lnk_cnt != links:
        print('INODE ' + str(ino_num) + ' HAS ' + str(links)
               + ' LINKS BUT LINKCOUNT IS ' + str(linkcount))
        is_consistent = False



# check for unreferenced and misallocated blocks
for blk in range (1, superblock.blk_cnt + 1):
    
    if blk in bfree:
        if blk in block_dict:
            is_consistent = False
            print('ALLOCATED BLOCK ' + str(blk) + ' ON FREELIST')
        
    elif (
          blk not in block_dict
          and blk not in reserved_blocks
        ):
         is_consistent = False
         print('UNREFERENCED BLOCK ' + str(blk))
        
# Check for references to unallocated inodes
for ino in ref_inode:
    if ino in ifree and ino in inode_dict_lr:
        dir_name = ref_inode[ino]
        parent_inode = inode_dict_lr[ino]
        print('DIRECTORY INODE ' + str(parent_inode)
               + ' NAME ' + dir_name[0:len(dir_name)- 2]
               + "' UNALLOCATED INODE " + str(x))
        is_consistent = False


# compare list of allocated/unallocated inodes with the free inode bitmaps
for ino in range(1, superblock.ino_cnt + 1):
    if (
        ino not in ifree
        and ino not in inode_dict_lnk_cnt
        and ino not in inode_dict_lr
        and ino not in reserved_inodes
      ):
        is_consistent = False
        print('UNALLOCATED INODE ' + str(ino) + ' NOT ON FREELIST')
    elif (
          ino in inode_dict_lnk_cnt
          and ino in ifree
        ):
         is_consistent = False
         print('ALLOCATED INODE ' + str(ino) + ' ON FREELIST')


# duplicate block entries
for blk in block_dict:
    if len(block_dict[blk]) > 1: # if the block has been referenced multiple times
        damaged = True
        for ref in block_dict[blk]: # ref: [inode number, offset, level]
            level = int(ref[2])
            if level == 0:
                strlvl = ""
            elif level == 1:
                strlvl = " INDIRECT"
            elif level == 2:
                strlvl = " DOUBLE INDIRECT"
            elif level == 3:
                strlvl = " TRIPLE INDIRECT"

            # print out messages for all references
            print('DUPLICATE' + strlvl + ' BLOCK '
                    + str(block) + ' IN INODE '
                    + str(ref[0]) + ' AT OFFSET '
                    + str(ref[1]) )



# directory links
for parent in inode_dict_parents:
    if parent == 2 and inode_dict_parents[parent] == 2:
        continue
    elif parent == 2:
        is_consistent = False
        print("DIRECTORY INODE 2 NAME '..' LINK TO INODE "
               + str(inode_dict_parents[parent]) + " SHOULD BE 2")
    
    elif parent not in inode_dict_lr:
        is_consistent = False
        print("DIRECTORY INODE " + str(inode_dict_parents[parent])
                 + " NAME '..' LINK TO INODE " + str(parent)
                 + " SHOULD BE " + str(inode_dict_parents[parent]) )
    
    elif inode_dict_parents[parent] != inode_dict_lr[parent]:
        is_consistent = False
        print("DIRECTORY INODE " + str(parent)
                + " NAME '..' LINK TO INODE " + str(parent)
                + " SHOULD BE " + str(inode_dict_lr[parent]) )
                

if is_consistent:
    exit(0)
else:
    exit(2)
