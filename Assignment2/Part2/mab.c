#include "mab.h"

MabPtr container[BLOCK_COUNT];

MabPtr root, root_temp, new_node_left, new_node_right, allocate_block;
bool found_node = false;
bool mem_freed = false;

int 
norm_mem_request(int mem_request) {
    if (mem_request > MAX_ALLOC) {
        return -1;
    }
    int actual_allocated_mem = 0;
    int level = 0;
    if (mem_request <= MIN_ALLOC) {
        actual_allocated_mem = MIN_ALLOC;
    } else {
        level = ceil(log2(mem_request));
        actual_allocated_mem = pow(2, level);
    }

    return actual_allocated_mem;
}

MabPtr
init_mem_block (int size, int offset_address) {
    MabPtr  new_block = malloc(sizeof(Mab));
    new_block -> size = size;
    new_block -> offset_address = offset_address;
    new_block -> next = NULL;
    new_block -> prev = NULL;
    new_block -> parent_block = NULL;
    new_block -> left_child_block = NULL;
    new_block -> right_child_block = NULL;
    new_block -> allocated = false;
    new_block -> remove = false;

    return new_block;
}

void 
print_tree() {
    for (int i = 0; i < BLOCK_COUNT; i++) {
        printf("Container %d; ", i);
        MabPtr temp = container[i];
        while (temp != NULL) {
            printf("%d ", temp->allocated);
            temp = temp->next;
        }
        printf("\n");
    }
    printf("\n");
}

MabPtr 
init_mem_system()
{
    root = init_mem_block(MAX_ALLOC, 0);
    root->level = BLOCK_COUNT - 1;
    container[BLOCK_COUNT - 1] = root;
    root_temp = root;
    return root;
}

void 
search_remove(int level) {

    if (container[level] == NULL) {
        return;
    }
    // find node to remove
    while (container[level]->remove != true) {
        container[level] = container[level]->next;
    }

    if (container[level]->prev != NULL) {
        container[level]->prev->next = container[level]->next;
        if (container[level]->next != NULL) {
            container[level]->next->prev = container[level]->prev;
        }
    } else {
        if (container[level] -> next != NULL) {
            container[level] = container[level] -> next;
            container[level]->prev = NULL;
        } else {
            container[level] = NULL;
        }
    }
}

MabPtr 
mem_split(MabPtr mem_block, int mem_request) {

    if (mem_block == NULL) {
        return NULL;
    } else if (mem_block->size == 8) {
        return NULL;
    }

    if (mem_block->right_child_block==NULL && 
        mem_block->left_child_block==NULL && 
        mem_block->size > mem_request && 
        mem_block-> allocated == false &&
        found_node == false) 
        {
            MabPtr  new_node_left = init_mem_block(mem_block->size/2, mem_block->offset_address);
            MabPtr  new_node_right = init_mem_block(mem_block->size/2, mem_block->offset_address + mem_block->size/2);

            #ifndef DEBUG_PRINT
                printf("left address: %d\n", new_node_left->offset_address);
                printf("right address: %d\n", new_node_right->offset_address);
            #endif

            /*link parent and child nodes, then delete parent from free list*/
            new_node_left->parent_block = mem_block;
            new_node_right->parent_block = mem_block;
            mem_block->left_child_block = new_node_left;
            mem_block->right_child_block = new_node_right;
            mem_block->remove = true;
        

            int parent_level = mem_block->level;
            #ifndef DEBUG_PRINT
                printf("%d\n", parent_level);
            #endif

            search_remove(parent_level);
            
            // inserting two nodes into free list one level down
            int child_level = parent_level - 1;

            #ifndef DEBUG_PRINT
            printf("%d\n", child_level);
            #endif

            new_node_left -> level = child_level;
            new_node_right -> level = child_level;

            // associate child nodes with other nodes in the container
            if (container[child_level] != NULL) {
                while (container[child_level] -> next != NULL) {
                    container[child_level] = container[child_level] -> next;
                }

                container[child_level] -> next = new_node_left;
                new_node_left -> prev = container[child_level];
                new_node_left -> next = new_node_right;
                new_node_right -> prev = new_node_left;
                new_node_right -> next = NULL;
            } else {
                container[child_level] = new_node_left;
                container[child_level] -> prev = NULL;
                container[child_level] -> next = new_node_right;
                new_node_right -> prev = new_node_left;
                new_node_right -> next = NULL;
            }

            #ifndef DEBUG_PRINT
                printf("address: %d\n", container[child_level] -> offset_address);
                printf("next address: %d\n", container[child_level] -> next -> offset_address);
                printf("left size: %d\n", new_node_left -> size);
                printf("right size: %d\n", new_node_right -> size);
                printf("mem_request: %d\n", mem_request);
            #endif
            

            if (new_node_left -> size == mem_request) {
                found_node = true;
                #ifndef DEBUG_PRINT
                    printf("test\n");
                #endif
                allocate_block =  new_node_left;
                return allocate_block;
            }
        }

    // if unable to split, split child nodes
    if (found_node == false) {
        #ifndef DEBUG_PRINT
        printf("test2\n");
        #endif
        if (mem_split(mem_block -> left_child_block, mem_request) && !found_node) {
            #ifndef DEBUG_PRINT
            printf("test3\n");
            #endif
            if (mem_split(mem_block -> right_child_block, mem_request)  && !found_node){
                #ifndef DEBUG_PRINT
                printf("test4\n");
                #endif
                return NULL;
            }
        }
    } 
    return allocate_block;
}

MabPtr
mem_alloc(MabPtr root_node, int mem_request) {

    //normalize memory request
    int allocated_mem = norm_mem_request(mem_request);

    if (allocated_mem < 0) {
        printf("Unable to allocate; memory request exceeds maximum allocation size\n");
        return NULL;
    }

    int level = log2(allocated_mem) - MIN_ALLOC_LOG2;

    #ifndef DEBUG_PRINT
    printf("level: %d\n", level);
    #endif
    allocate_block = NULL;

    // free memory block if that size is available
    if (container[level] != NULL) {
        
        if (container[level]->size == allocated_mem){
            allocate_block = container[level];

            if (container[level] -> next != NULL) {
                container[level] = container[level]-> next;
                container[level] -> prev = NULL;
            } else 
                container[level] = NULL;

            allocate_block -> next = NULL;
            allocate_block -> prev = NULL;
        }   
    }
    // no free memory block of that size is available
    else {
        // split block subroutine
        found_node = false;
        root = root_temp;
        mem_split(root, allocated_mem);

        if (allocate_block != NULL) {
            //allocate block - to be removed from free container
            allocate_block -> remove = true;
        } else {
            // printf("Unable to allocate; unable to find block of appropriate size\n");
            return NULL;
        }
    }

    // if node not found and unable to split
    if (allocate_block == NULL) {
        return NULL;
    }
    // remove from free container

    if (allocate_block -> remove == true) {
        int block_level = allocate_block -> level;
        

        #ifndef DEBUG_PRINT
        printf("toremove1: %d\n", container[block_level] -> size);
        printf("toremove1: %d\n", allocate_block->size); 
        #endif

        printf("%d\n", block_level);
        search_remove(block_level);
    }
    allocate_block -> allocated = true;
    return allocate_block;
}

// recursively check if node can be merged from leaf to root
MabPtr 
mem_merge (MabPtr mem_block) {

    if (mem_block == NULL) {
        return mem_block;
    }

    // check merge conditions
    if (mem_block->parent_block != NULL && // check not root
        mem_block->parent_block->left_child_block->left_child_block==NULL && // check left child block doesnt not have children
        mem_block->parent_block->left_child_block->right_child_block==NULL && // check left child block doesnt not have children
        mem_block->parent_block->right_child_block->left_child_block==NULL && // check right child block doesnt not have children
        mem_block->parent_block->right_child_block->right_child_block==NULL) // check left child block doesnt not have children
        {

            // able to merge
            if (mem_block->parent_block->left_child_block->allocated==false &&
                mem_block->parent_block->right_child_block->allocated==false)
            {   
                
                // if mem_block is a left child
                if (mem_block->parent_block->left_child_block == mem_block) {
                    // remove the right sibling
                    MabPtr to_free = mem_block->parent_block->right_child_block;
                    to_free->remove = true;

                    int right_child_level = to_free->level;

                    search_remove(right_child_level);
                    free(to_free);
                }
                
                // if mem_block is a right child
                else if (mem_block->parent_block->right_child_block == mem_block) {
                    // remove the left sibling
                    MabPtr to_free = mem_block->parent_block->left_child_block;
                    to_free->remove = true;

                    int left_child_level = to_free->level;

                    search_remove(left_child_level);
                    free(to_free);
                }

                mem_block->parent_block->left_child_block = NULL;
                mem_block->parent_block->right_child_block = NULL;

                return mem_merge(mem_block->parent_block);

            } else {
                
                // inserting node into free list
                if (mem_block->parent_block->left_child_block->allocated == true) {

                    int right_child_level = mem_block->parent_block->right_child_block->level;

                    if (container[right_child_level] != NULL) {
                        while (container[right_child_level]->next != NULL) {
                            container[right_child_level] = container[right_child_level]->next;
                        }
                        container[right_child_level]->next = mem_block->parent_block->right_child_block;
                        mem_block->parent_block->right_child_block->prev = container[right_child_level];
                    } else {
                        container[right_child_level] = mem_block->parent_block->right_child_block;
                        container[right_child_level]->prev = NULL;
                        container[right_child_level]->next = NULL;
                    }
                } else if (mem_block->parent_block->right_child_block->allocated == true) {

                    int left_child_level = mem_block->parent_block->left_child_block->level;

                    if (container[left_child_level] != NULL) {
                        while (container[left_child_level]->next != NULL) {
                            container[left_child_level] = container[left_child_level]->next;
                        }
                        container[left_child_level]->next = mem_block->parent_block->left_child_block;
                        mem_block->parent_block->left_child_block->prev = container[left_child_level];
                    } else {
                        container[left_child_level] = mem_block->parent_block->left_child_block;
                        container[left_child_level]->prev = NULL;
                        container[left_child_level]->next = NULL;
                    }
                }
            }
        } else {
            // if node is a root node
            int level = mem_block->level;
    
            if (container[level] != NULL){
                while (container[level]->next != NULL) {
                    container[level] = container[level]->next;
                }
                container[level]->next = mem_block;
                mem_block->prev = container[level];
            } else {
                container[level] = mem_block;
                container[level]->prev = NULL;
                container[level]->next = NULL;
            }   
        }
    
    return NULL;
}

MabPtr
mem_free (MabPtr mem_block) {

    if (mem_block == NULL) {
        return mem_block;
    }

    // no child blocks
    if (mem_block->left_child_block == NULL && mem_block->right_child_block == NULL) {
        mem_block->allocated = false;
        mem_freed = true;

        
        if (mem_block->parent_block != NULL) {

            /*
            check if sibling node is unallocated and both current and sibling 
            have no child blocks
            */ 

           if (mem_block->parent_block->left_child_block->allocated == false && 
            mem_block->parent_block->right_child_block->allocated == false &&
            mem_block->parent_block->left_child_block->left_child_block==NULL && // check left child block doesnt not have children
            mem_block->parent_block->left_child_block->right_child_block==NULL && // check left child block doesnt not have children
            mem_block->parent_block->right_child_block->left_child_block==NULL && // check right child block doesnt not have children
            mem_block->parent_block->right_child_block->right_child_block==NULL) {

                // merge nodes

                // check if mem_block is left child
                if (mem_block->parent_block->left_child_block == mem_block) {
                    
                    // remove right child from free list 
                    MabPtr to_free = mem_block->parent_block->right_child_block;
                    to_free->remove = true;

                    int right_child_level = to_free->level;
                    search_remove(right_child_level);

                } else if (mem_block->parent_block->right_child_block == mem_block){
                    // remove left child from free list
                    MabPtr to_free = mem_block->parent_block->left_child_block;
                    to_free->remove = true;

                    int left_child_level = to_free->level;
                    
                    search_remove(left_child_level);
                }

                mem_block->parent_block->left_child_block = NULL;
                mem_block->parent_block->right_child_block = NULL;

                if (mem_block->parent_block != NULL) {
                    return mem_merge(mem_block->parent_block);
                }

           } else {
                // unable to merge, add to free list
                int level = mem_block->level;

                if (container[level] != NULL) {
                    while (container[level]->next != NULL) {
                        container[level] = container[level]->next;
                    }

                    container[level]->next = mem_block;
                    mem_block->prev = container[level];
                } else {
                    container[level] = mem_block;
                    container[level]->prev = NULL;
                    container[level]->next = NULL;
                }
            }
        } 
    }

    return mem_block;
}

// int main() {
//     init_mem_system();

//     MabPtr a, *b, *c, *d;

//     a = mem_alloc(8);
//     c = mem_alloc(8);
//     d = mem_alloc(8);
//     b = mem_alloc(8);

//     mem_free(a);
//     mem_free(b);
//     mem_free(c);
//     mem_free(d);

//     print_tree();

//     a = mem_alloc(2048);
//     mem_free(a);
    
//     a = mem_alloc(8);
//     c = mem_alloc(8);
//     d = mem_alloc(8);
//     b = mem_alloc(8);

//     mem_free(a);
//     mem_free(b);
//     mem_free(c);
//     mem_free(d);

//     print_tree();
    
//     return 0;
// }