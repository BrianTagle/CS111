#!/usr/bin/env python3
#NAME: Brian Tagle
#EMAIL: taglebrian@gmail.com
#ID: 604907076

import sys
import csv
import collections

consistent = True


class BLOCK:
    def __init__(self, level, block_num, inode_num, offset):

        self.level = level
        self.block_num = block_num
        self.inode_num = inode_num
        self.logical_offset = offset
        self.duplicate = 0


class SUPERBLOCK:
    def __init__(self, field):
        self.s_blocks_count = int(field[1])
        self.s_inodes_count = int(field[2])
        self.s_block_size = int(field[3])
        self.s_inode_size = int(field[4])


class GROUP:
    def __init__(self, field):

        self.bg_inode_table = int(field[8])

class INODE:
    def __init__(self, field):
        self.i_num = int(field[1])
                
        self.i_links_count = int(field[6])

        self.i_block_list = []
        if field[2] != 's':
            for i in range(12, 24):
                self.i_block_list.append(int(field[i]))

        self.i_pointer_list = []
        if field[2] != 's':
            for i in range(24, 27):
                self.i_pointer_list.append(int(field[i]))


class DIRENT:
    def __init__(self, field):
        self.de_parent_inode = int(field[1])

        self.de_inode_num = int(field[3])

        self.de_name = str(field[6])

class INDIRECT:
    def __init__(self, field):
        self.id_inode_num = int(field[1])
        self.id_indirection_level = int(field[2])
        self.id_offset = int(field[3])

        self.id_ref_block_num = int(field[5])



def set_inconsistent():
    consistent = False
    
def block_audit(list_free_blocks, list_free_inodes, list_indirect, super_block, inode_table):


    block_referenced_dict = {}
    valid_block = inode_table + super_block.s_inode_size * super_block.s_inodes_count / super_block.s_block_size #first non reserved block

    for inode in list_free_inodes:
        logical_offset = 0
        for block in inode.i_block_list:
            if block != 0:
                if block < 0 or block > (super_block.s_blocks_count):
                    print('INVALID BLOCK {} IN INODE {} AT OFFSET {}'.format(block, inode.i_num, logical_offset), file=sys.stdout)
                    set_inconsistent()
                if block < valid_block:
                    print('RESERVED BLOCK {} IN INODE {} AT OFFSET {}'.format(block, inode.i_num, logical_offset), file=sys.stdout)
                    set_inconsistent()
                elif block not in block_referenced_dict:
                    block_referenced_dict[block] = BLOCK('', block, inode.i_num, logical_offset)
                    
                        
                else:
                    print('DUPLICATE {} {} IN INODE {} AT OFFSET {}'.format("BLOCK", block, inode.i_num, logical_offset), file=sys.stdout)
                    set_inconsistent()
                    
                    duplicate_block = block_referenced_dict[block]
                    if duplicate_block.duplicate == 0:
                        print('DUPLICATE {} {} IN INODE {} AT OFFSET {}'.format(duplicate_block.level + "BLOCK", duplicate_block.block_num, duplicate_block.inode_num, duplicate_block.logical_offset), file=sys.stdout)
                        #set_inconsistent()
                        block_referenced_dict[block].duplicate = 1
                    
                #
            logical_offset = logical_offset + 1

        for i in range(3): 
            if i == 0:
                level = "INDIRECT"
                logical_offset = 12
            elif i == 1:
                level = "DOUBLE INDIRECT"
                logical_offset = 268
            elif i == 2:
                level = "TRIPLE INDIRECT"
                logical_offset = 65804

            Block_pointer =  inode.i_pointer_list[i]
            if Block_pointer != 0:
                if Block_pointer < 0 or Block_pointer > (super_block.s_blocks_count ) : #can't be at total block amount
                    print('INVALID {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level, Block_pointer, inode.i_num, logical_offset), file=sys.stdout)
                    set_inconsistent()
                if Block_pointer < valid_block:
                    print('RESERVED {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level, Block_pointer, inode.i_num, logical_offset), file=sys.stdout)
                    set_inconsistent()
                elif Block_pointer not in block_referenced_dict:
                    block_referenced_dict[Block_pointer] = BLOCK(level, Block_pointer, inode.i_num, logical_offset)
                         
                else:
                    print('DUPLICATE {} {} IN INODE {} AT OFFSET {}'.format(level+ " BLOCK", Block_pointer, inode.i_num, logical_offset), file=sys.stdout)
                    set_inconsistent()

                    duplicate_block = block_referenced_dict[Block_pointer]
                    if duplicate_block.duplicate == 0:
                        print('DUPLICATE {} {} IN INODE {} AT OFFSET {}'.format(duplicate_block.level + "BLOCK", duplicate_block.block_num, duplicate_block.inode_num, duplicate_block.logical_offset), file=sys.stdout)
                        #set_inconsistent()
                        block_referenced_dict[Block_pointer].duplicate = 1


    for i in range(len(list_indirect)):
        indirectionlevel = list_indirect[i].id_indirection_level
        if indirectionlevel == 0:
            level = "INDIRECT"
        elif indirectionlevel == 1:
            level = "DOUBLE INDIRECT"
        elif indirectionlevel == 2:
            level = "TRIPLE INDIRECT"

        logical_offset = list_indirect[i].id_offset
        
        if list_indirect[i].id_ref_block_num != 0:
            if list_indirect[i].id_ref_block_num < 0 or list_indirect[i].id_ref_block_num > ( super_block.s_blocks_count ):
                print('INVALID {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level, list_indirect[i].id_ref_block_num, list_indirect[i].id_inode_num, logical_offset), file=sys.stdout)
                set_inconsistent()
            if list_indirect[i].id_ref_block_num < valid_block:
                print('INVALID {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level, list_indirect[i].id_ref_block_num, list_indirect[i].id_inode_num, logical_offset), file=sys.stdout)
                set_inconsistent()
            elif list_indirect[i].id_ref_block_num not in block_referenced_dict:
                block_referenced_dict[list_indirect[i].id_ref_block_num] = BLOCK(level, list_indirect[i].id_ref_block_num, list_indirect[i].id_inode_num, logical_offset)
                
                    
            else:
                print('DUPLICATE {} {} IN INODE {} AT OFFSET {}'.format(level+ " BLOCK", list_indirect[i].id_ref_block_num, list_indirect[i].id_inode_num, logical_offset), file=sys.stdout)
                set_inconsistent()
                
                duplicate_block = block_referenced_dict[list_indirect[i].id_ref_block_num]
                if duplicate_block.duplicate == 0:
                    print('DUPLICATE {} {} IN INODE {} AT OFFSET {}'.format(duplicate_block.level + "BLOCK", duplicate_block.block_num, duplicate_block.inode_num, duplicate_block.logical_offset), file=sys.stdout)
                    #set_inconsistent()
                    block_referenced_dict[list_indirect[i].id_ref_block_num].duplicate = 1


    for block in range(int(valid_block), super_block.s_blocks_count ):

        if block not in block_referenced_dict and block not in list_free_blocks:
            print('UNREFERENCED BLOCK {}'.format(block), file=sys.stdout)
            set_inconsistent()
        if block in block_referenced_dict and block in list_free_blocks:
            print('ALLOCATED BLOCK {} ON FREELIST'.format(block), file=sys.stdout)
            set_inconsistent()
 
def inode_audit(inode_list, i_bitmap, super_block):
    #catches fringe cases that deal with inodes in reserved space
    for bit in i_bitmap:
        for inode in inode_list:
            if bit == inode.i_num: #on freelist but also allocated
                print('ALLOCATED INODE {} ON FREELIST'.format(inode.i_num), file=sys.stdout)
                set_inconsistent()
                
    for i in range(11, super_block.s_inodes_count): #there are 10 reserved inodes
        allocated = False
        free = False
        for inode in inode_list:
            if i == inode.i_num:  #and i != inode.inode_num
                allocated = True
        for numFree in i_bitmap:
            if i == numFree:
                free = True
                
        if not allocated and not free:
            print('UNALLOCATED INODE {} NOT ON FREELIST'.format(i), file=sys.stdout)
            set_inconsistent()



def directory_audit(dirent_list, inode_list, inode_count):
    inode_link_count = collections.defaultdict(lambda : 0) #{}

    #for i in range(1, super_block.total_inodes+1):
    #    inode_link_count_dict[i] = 0

    
    
    for dirent in dirent_list:

                

        if dirent.de_name ==  "'.'" and dirent.de_parent_inode != dirent.de_inode_num:
            print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(dirent.de_parent_inode, dirent.de_name, dirent.de_inode_num, dirent.de_parent_inode), file=sys.stdout)
            set_inconsistent()
        if dirent.de_parent_inode == 2 and dirent.de_name == "'..'" and dirent.de_parent_inode != dirent.de_inode_num:
            print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(dirent.de_parent_inode, dirent.de_name, dirent.de_inode_num, dirent.de_parent_inode), file=sys.stdout)
            set_inconsistent()
        if dirent.de_parent_inode != 2 and dirent.de_name == "'..'":
            for parentDirEnt in dirent_list: #possibleParents:
                if dirent.de_inode_num != parentDirEnt.de_parent_inode and parentDirEnt.de_inode_num == dirent.de_parent_inode and parentDirEnt.de_parent_inode != dirent.de_parent_inode:
                    print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(dirent.de_parent_inode, dirent.de_name, dirent.de_inode_num, parentDirEnt.de_parent_inode), file=sys.stdout)
                    set_inconsistent()

                    
        allocated = False
        for inode in inode_list:
            if dirent.de_inode_num == inode.i_num:
                allocated = True    
        if dirent.de_inode_num< 1 or dirent.de_inode_num > inode_count:
            print('DIRECTORY INODE {0} NAME {1} INVALID INODE {2}'.format(dirent.de_parent_inode, dirent.de_name, dirent.de_inode_num), file=sys.stdout)
            set_inconsistent()
        elif allocated == False:
            print('DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}'.format(dirent.de_parent_inode, dirent.de_name, dirent.de_inode_num), file=sys.stdout)
            set_inconsistent()
        else:

            inode_link_count[dirent.de_inode_num] += 1

    for inode in inode_list:

            if inode.i_links_count != inode_link_count[inode.i_num]:
                print('INODE {} HAS {} LINKS BUT LINKCOUNT IS {}'.format(inode.i_num, inode_link_count[inode.i_num], inode.i_links_count), file=sys.stdout)
                set_inconsistent()



def main():
    if len(sys.argv) != 2:
        print("Error, requries one argument of a csv file", file=sys.stderr)
        sys.exit(1)
        
    super_block = None
    group = None
    b_bitmap = []
    i_bitmap = []
    inode_list = []
    dirent_list = []
    indirect_list = []


    try:
        with open(sys.argv[1]) as csv_file:
            csv_reader = csv.reader(csv_file)
            for row in csv_reader:
                if row[0] == "SUPERBLOCK":
                    super_block = SUPERBLOCK(row)
                if row[0] == "GROUP":
                    group = GROUP(row)

                if row[0] == "BFREE":
                    b_bitmap.append(int(row[1]))
                if row[0] == "IFREE":
                    i_bitmap.append(int(row[1]))
                    
                if row[0] == "INODE":
                    inode_list.append(INODE(row))
                if row[0] == "DIRENT":
                    dirent_list.append(DIRENT(row))
                if row[0] == "INDIRECT":
                    indirect_list.append(INDIRECT(row))

    except IOError:
        print("Error, file was not readable", file=sys.stderr)
        sys.exit(1)

    block_audit(b_bitmap, inode_list, indirect_list, super_block, group.bg_inode_table)
    inode_audit(inode_list, i_bitmap, super_block)
    directory_audit(dirent_list, inode_list, super_block.s_inodes_count)

    if consistent:
        sys.exit(0)
    else:
        sys.exit(2)

if __name__ == '__main__':
    main()
    
