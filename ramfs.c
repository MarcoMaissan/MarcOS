#include <ramfs.h>
#include <alloc.h>

//
//
//Marco Maissan
//0949830
//TI2B
//
//

static struct file *head;
bool debugram = false;

/* Seek for files by 'name' */
struct file *ramfs_seek(const char *name)
{
    //Go to head, compare string and return if strings are equal
    struct file *temp = head;
    while(temp != NULL){
        if(strcmp(temp->name, name) == 0){
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

/* Return the head of the file list */
struct file *ramfs_readdir(void)
{
    return head;
}

/*
 * Read from a file into 'data' (which is automatically (re)allocated)
 * Return:
 *  # of bytes read
 * -1 when out of memory
 */
ssize_t ramfs_read(struct file *fp, char **data)
{
     char* mem = (char*)malloc(fp->size);
     strcpy(mem, fp->data);
     *data = mem; 
}

/*
 * Write 'n' bytes to a file from 'data'
 * Return:
 *  # of bytes written
 * -1 when out of memory
 */
ssize_t ramfs_write(struct file *fp, const char *data, size_t n)
{
    const char* mem = (const char*)malloc(n);
    strcpy(mem, data);
    fp->data = mem;
    fp->size = n;
}

/*
 * Create a file with 'name'
 * Return:
 *  0 on success
 * -1 when out of memory
 * -2 when file with 'name' already exists
 * -3 when an invalid 'name' is entered
 */
int ramfs_create(const char *name)
{
    //allocate new file
    void* allocatedfile = malloc(sizeof(struct file));

    //set file properties
    struct file* newfile = (struct file*) allocatedfile;
    strcpy(newfile->name, name);
    newfile->prev = newfile->next = NULL;
    newfile->data = NULL;
    newfile->size = 0;

    //if head exists, set head. Otherwise set as last from list
    if(head == NULL){
        head = newfile;
    }else{
        struct file *temp = head;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = newfile;
        newfile->prev = temp;
    }

    if (debugram == true) { 
        ls();
    }
    return 0;
}

/* Remove a file and it's data */
void ramfs_remove(struct file *fp)
{
    //free the memory block
    free((void*)fp);
    if(fp->prev == NULL && fp->next == NULL){
        //only item in list
        head = NULL;
    }
    else if(fp->prev == NULL){
        //its the head, so change it
        head = fp->next; //either null or next block
    }
    else if(fp->next == NULL){
        //end of list -> set next of previous to null
        fp->prev->next = NULL;
    }else{
        //the previous' items next item is fp->next
        fp->prev->next = fp->next;
        //the next item's previous one will be fp->prev
        fp->next->prev = fp->prev;
    }
}

void ls(){
    struct file *temp = head;
    while (temp != NULL){
        printf("\n%s",temp->name);
        if(debugram) {printf("\nprev %p, mine %p, next %p",temp->prev, temp, temp->next);} //Debugging purposes...
        temp = temp->next;
    }   
}

void rm(char* file){
    //Remove file command...
    struct file *f = ramfs_seek(file);
    if(f != NULL){
    ramfs_remove(f);
    }
}

void touch(const char *name){
    //make new file with name
   if(ramfs_seek(name) == NULL){
    ramfs_create(name);
    //find file and fill with empty data
    struct file *f = ramfs_seek(name);
    ramfs_write(f, "\0", sizeof("\0"));
   }else{
    printf("\nFile already exists!");
   }


}

void cat(const char *name){
    //Display the contents of a file
    struct file *f = ramfs_seek(name);
    if(f != NULL){
       printf("\n%s", f->data);
    }
}

void testdata(){
    //enter some testdata to experiment with (accessed as "td" in terminal)
    printf("\n");
    struct file *f1 = ramfs_seek("c");
    ramfs_write(f1, "Heyyyy", sizeof("Heyyy")+1);
    
    //read data test
    
    char *datapointer;
    ramfs_read(f1, &datapointer);
    printf("%s", datapointer);
}
