/*
 *
 * BertOS - Memory allocator
 * src/alloc.c
 *
 * Copyright (C) 2019 Bastiaan Teeuwen <bastiaan@mkcl.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 */

#include <alloc.h>

static struct block *head = NULL;

//Not a pointer!
extern uintptr_t kern_end;
bool debug = false;

static void *block_alloc(size_t n)
{
    //Search for last block
    if(head == NULL){
        //If no block exists yet, generate a new one
        //Generate the block at the kern_end address
        //Set address to the blocksize and cast as void* to get correct size
        struct block *bptr = &kern_end;
        bptr->addr = (void*)bptr + sizeof(struct block);
        bptr->size = n;
        bptr->used = true;
        bptr->next = NULL;
        head = bptr;
        if(debug) printf("Created head at address = %p\n", head);
        return head->addr;
    }else{
        //Get the last from the list
        //Make a temporary pointer pointing to the last structs addr + size_t
        //This will return the entry point of the new block
        struct block* last = getlast(head);
        struct block *bptr = (void*)last->addr + last->size;

        bptr->addr = (void*)bptr + sizeof(struct block);
        bptr->size = n;
        bptr->used = true;
        bptr->next = NULL; 

        last->next = bptr;

        if(debug){
            printf("Lasts next block will be at %p\n", last->next);
            printf("Next writable memory space will be at %p\n", last->next->addr);}

        return last->next->addr;
    }
}

struct block *getlast(struct block *head){
    //Skip to last element recursively
    if(debug)printf("Entering getlast() recursively\n");
    if(head->next == NULL){
        return head;   
    }else{
        return getlast(head->next);
    }
} 

static void block_free(struct block *bptr)
{
   if(debug) printf("Freeing the value, now it is seen as %d\n", bptr->used);
    bptr->used = false;
    if(debug) printf("The value changed to %d\n", bptr->used);
}

static void *block_get(size_t n)
{
    struct block *ptr = head;
    while(true){
       // printf("ptr is now %p\n", ptr);
        //while true is used since it will return sometime anyway
        if(!ptr){
            //if no head exists
            return block_alloc(n);
        }else{
            if(ptr->size == n && ptr->used == false){
                ptr->used = true;
                //regenerate address since it might have been lost on double backspace
                ptr->addr = (void*)ptr + sizeof(struct block);
                //if an unused block is found, return its address
                return ptr->addr;
            }else{
                if(ptr->next != NULL){
                    //go to next block
                    ptr = ptr->next;
                }else{
                    void* block = block_alloc(n);
                    //if no new blocks are available, make a new block
                    return block;
                }
            }
        }
    }
}

void *malloc(size_t n)
{
    //pretty self-explanatory
    if(head == NULL){
        return block_alloc(n);
    }
    else{
        if(n > 0){
            return block_get(n);        
        }else{
            return NULL;
        }
    }

}

void free(void *ptr)
{
    //receives writable memory space addr
    if(ptr != NULL){
        //To remove the block of an address, 
        //generate its address by subtracting the block size from the add
        block_free(ptr - sizeof(struct block));
    }
}


void *realloc(void *ptr, size_t n)
{
    /* TODO */
}

void *calloc(size_t n, size_t m)
{
    /* TODO */
}
