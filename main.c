#include <alloc.h>
#include <sys/print.h>
#include <sys/vga.h>
#include <sys/ps2.h>
/*
 *
 * BertOS - I/O assignment
 * src/main.c
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
int stringlength = 0;
int lines = 0;

//Writable memory space start and end, NOT THE BLOCKS
int* stringstart = NULL;
int* stringend = NULL;
bool halted = false;
void main(void)
{
    vga_clear();
    prompt();
    //Pointer to start of sentence.
    //Use variable memory allocation since sentence length can vary
    //eallocate memory after each input of character
    while(!halted){
        char c = ps2_getch();

        if(c == '\b'){
            printf("%c", '\b');
            backspace();
        }
        else if(c == '\n'){
            enter();
        }
        else{
            stringend = malloc(sizeof(char));
            if(stringstart == NULL) stringstart = stringend;
            //set value of stringend to char
            *stringend = c;
            printf("%c", *stringend);
            //alloc new size and set stringend to writable memory space
            stringlength+=1; 
        }
    }
    printf("\nHalted!");
}

void printbuffer(){
    struct block *temp = (void*)stringstart - sizeof(struct block);
    for(int i = 0; i < stringlength; i++){
        int *c = (void*)temp + sizeof(struct block);
        char d = *c;
        printf("%c", d);
        temp = temp->next;
    }
}

void prompt(){printf("M> ");}

void enter(){
    if(stringlength > 0){
        if(checkcommand("halt", 4) == true){
            halted = true;
        }else if(checkcommand("clear", 5) == true){
            vga_clear();
            clear();
        }else{
            printf("\nCommand ");
            printbuffer();
            printf(" not found!\n");
            clear();
        }
    }
}

void checkcommand(char* command, int size){
    struct block *temp = (void*)stringstart - sizeof(struct block);
    for(int i = 0; i < size; i++){
        int *c = temp->addr;
        char d = *c;
        //printf("*c = %c && command[i] = %c\n", *c, command[i]);
        if(d != command[i]) 
        {   
            return false;
        }
        temp = temp->next; 
    }
    printf("\n");
    return true;
}

void clear(){
    prompt();
    struct block *temp = (void*)stringstart - sizeof(struct block);
    freeBuffer(temp);
}

void freeBuffer(struct block *head){
    //Skip to last element recursively
    free((void*) head + sizeof(struct block));
    stringend = stringstart;
    stringlength = 0;
    if(head->next == NULL){
        return;   
    }else{
        freeBuffer(head->next);
    }
} 

void backspace(){
    //printf("%c", '\b');
    if(stringlength > 1){
        struct block *temp = (void*)stringstart - sizeof(struct block);

        for(int i = 0; i < stringlength-1; i++){
            temp = temp->next;
        }
        char *letter = temp->addr;
        //printf("It is going to free the block with letter %c\n", *letter);

        free(temp->addr);
        if(stringlength > 0){
            stringend = (void*)temp - sizeof(char);
        }
        stringlength -= 1;
    }else{
        struct block *temp = (void*)stringstart - sizeof(struct block);
        free(temp->addr);
        stringlength = 0;
    }
}
