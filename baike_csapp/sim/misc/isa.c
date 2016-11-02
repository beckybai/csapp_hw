#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// added head file
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/file.h>
#include <assert.h>
//#include "lock.h"
#include "isa.h"



/* Are we running in GUI mode? */
extern int gui_mode;

/* Bytes Per Line = Block size of memory */
#define BPL 32
#define ALGIN 0x07
id = -1;
int fd_lock;


struct {
    char *name;
    int id;
} reg_table[REG_ERR+1] = 
{
    {"%eax",   REG_EAX},
    {"%ecx",   REG_ECX},
    {"%edx",   REG_EDX},
    {"%ebx",   REG_EBX},
    {"%esp",   REG_ESP},
    {"%ebp",   REG_EBP},
    {"%esi",   REG_ESI},
    {"%edi",   REG_EDI},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_NONE},
    {"----",  REG_ERR}
};


/*F_LOCK ：给文件互斥加锁，若文件以被加锁，则会一直阻塞到锁被释放。

F_TLOCK ：同 F_LOCK ，但若文件已被加锁，不会阻塞，而回返回错误。

F_ULOCK ：解锁。

F_TEST ：测试文件是否被上锁，若文件没被上锁则返回 0 ，否则返回 -1 。
*/
void init_lock() {
    printf("initialize the lock\n");
    fd_lock = open(LOCK_FILE, O_RDWR | O_CREAT |O_TRUNC, S_IRUSR | S_IWUSR);
    printf("fd_lock%d,%d\n", fd_lock,ff);
}

int lock(mem_t m) {
//    printf("--- try to lock %s\n", LOCK_FILE); 
//    printf("lock %d",flock(fd_lock, LOCK_EX|LOCK_NB) );
    while (flock(fd_lock, LOCK_EX|LOCK_NB) != 0) {
      printf("%d : (lock by another program)%d\n",id,ff );
      receive_message(m);
      usleep(100);
    }
//    if(flock(fd_lock,LOCK_EX | LOCK_NB)!=0)
        printf( "lock_succeed\n");
    return 1;
}

void unlock() {
    if(flock(fd_lock,LOCK_UN)==0)
        printf("succeed unlock! \n");
    else
        printf("failed unlock\n");
}


reg_id_t find_register(char *name)
{
    int i;
    for (i = 0; i < REG_NONE; i++)
    if (!strcmp(name, reg_table[i].name))
        return reg_table[i].id;
    return REG_ERR;
}

char *reg_name(reg_id_t id)
{ 
    if (id >= 0 && id < REG_NONE)
    return reg_table[id].name;
    else
    return reg_table[REG_NONE].name;
}

/* Is the given register ID a valid program register? */
int reg_valid(reg_id_t id)
{
  return id >= 0 && id < REG_NONE && reg_table[id].id == id;
}

instr_t instruction_set[] = 
{
    {"nop",    HPACK(I_NOP, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"halt",   HPACK(I_HALT, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"rrmovl", HPACK(I_RRMOVL, F_NONE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* Conditional move instructions are variants of RRMOVL */
    {"cmovle", HPACK(I_RRMOVL, C_LE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovl", HPACK(I_RRMOVL, C_L), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmove", HPACK(I_RRMOVL, C_E), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovne", HPACK(I_RRMOVL, C_NE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovge", HPACK(I_RRMOVL, C_GE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovg", HPACK(I_RRMOVL, C_G), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* arg1hi indicates number of bytes */
    {"irmovl", HPACK(I_IRMOVL, F_NONE), 6, I_ARG, 2, 4, R_ARG, 1, 0 },
    {"rmmovl", HPACK(I_RMMOVL, F_NONE), 6, R_ARG, 1, 1, M_ARG, 1, 0 },
    {"mrmovl", HPACK(I_MRMOVL, F_NONE), 6, M_ARG, 1, 0, R_ARG, 1, 1 },
    {"addl",   HPACK(I_ALU, A_ADD), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"subl",   HPACK(I_ALU, A_SUB), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"andl",   HPACK(I_ALU, A_AND), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"xorl",   HPACK(I_ALU, A_XOR), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* arg1hi indicates number of bytes */
    {"jmp",    HPACK(I_JMP, C_YES), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"jle",    HPACK(I_JMP, C_LE), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"jl",     HPACK(I_JMP, C_L), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"je",     HPACK(I_JMP, C_E), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"jne",    HPACK(I_JMP, C_NE), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"jge",    HPACK(I_JMP, C_GE), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"jg",     HPACK(I_JMP, C_G), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"call",   HPACK(I_CALL, F_NONE),    5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"ret",    HPACK(I_RET, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"pushl",  HPACK(I_PUSHL, F_NONE) , 2, R_ARG, 1, 1, NO_ARG, 0, 0 },
    {"popl",   HPACK(I_POPL, F_NONE) ,  2, R_ARG, 1, 1, NO_ARG, 0, 0 },
    {"iaddl",  HPACK(I_IADDL, F_NONE), 6, I_ARG, 2, 4, R_ARG, 1, 0 },
    {"leave",  HPACK(I_LEAVE, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    /* this is just a hack to make the I_POP2 code have an associated name */
 //////////////////   {"pop2",   HPACK(I_POP2, F_NONE) , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"rmchange",  HPACK(I_RMCHANGE, F_NONE) , 6, R_ARG, 1, 1, M_ARG, 1, 0},
    /* For allocation instructions, arg1hi indicates number of bytes */
    {".byte",  0x00, 1, I_ARG, 0, 1, NO_ARG, 0, 0 },
    {".word",  0x00, 2, I_ARG, 0, 2, NO_ARG, 0, 0 },
    {".long",  0x00, 4, I_ARG, 0, 4, NO_ARG, 0, 0 },
    {NULL,     0   , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 }
};

instr_t invalid_instr =
    {"XXX",     0   , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 };

instr_ptr find_instr(char *name)
{
    int i;
    for (i = 0; instruction_set[i].name; i++)
    if (strcmp(instruction_set[i].name,name) == 0)
        return &instruction_set[i];
    return NULL;
}

/* Return name of instruction given its encoding */
char *iname(int instr) {
    int i;
    for (i = 0; instruction_set[i].name; i++) {
    if (instr == instruction_set[i].code)
        return instruction_set[i].name;
    }
    printf("++++++++++++++");//debug
    return "<bad>";
}


instr_ptr bad_instr()
{
    return &invalid_instr;
}

cache_set_t init_cache_mem() {
    cache_set_t result = (cache_set_t)malloc(sizeof(cache_set));
    memset(result,0,sizeof(cache_set));
    return result;
}

#define PTR ((byte_t*)(result -> f_share_mem_t))
file_mem_t init_file_mem() {
    init_lock();
//--    printf("prepare the share memory\n");
    file_mem_t result = (file_mem_t) malloc(sizeof(file_mem)); 
        
    if ((result->fd = open(SHARE_FILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
            printf("open error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
//!!! something needed here to judge if initialization is needed here.
    // initialize fd to 0
//    ftruncate(result->fd, SHARE_FILE);
//    byte_t zeros[SHARE_SIZE] = {0};
//    write(result->fd, zeros, sizeof(zeros));
    /*
        printf("what %d\n",result->fd);
        if (write(result->fd, str, SHARE_SIZE) == -1) {
            printf("write error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }   */

        // map into ff,memory
        if ((result -> f_share_mem_t = mmap(NULL, SHARE_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED| MAP_FILE, result->fd , 0)) == MAP_FAILED) {
        printf("mmap error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
        }

        if (close(result->fd) == -1) {
            printf("close error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        int i;

//        printf("ptrr in the function%d,%d\n",ptrr[0],ptrr[(SWAP_SIZE)]);

        printf("the begin point is %d\t%c\n\n",PTR,result -> f_share_mem_t[0]);
        printf("the size of share_mem is %d\n",sizeof(result->f_share_mem_t));
        printf("PRT [0]is %d\n",PTR[0] );
        printf("the %d size is %d now. \n",ff,PTR[SWAP_SIZE]);
        PTR[SWAP_SIZE]++;
        id = PTR[SWAP_SIZE];
        printf("(%d)\n", id);

//        sleep(1);

/*
// wait until anthor pragram came in...
        while(PTR[SWAP_SIZE]!=2)
        {
            sleep(2);
            printf("wait for another program\n");
        }


*/

    // put the half of the share memory to be the notice of total users in this system.
/*        if(flock(fd_lock,LOCK_EX | LOCK_NB)==0){

//            ftruncate(result->fd, SHARE_FILE);
//            byte_t zeros[SHARE_SIZE] = {0};
            //    write(result->fd, zeros, sizeof(zeros));
            printf("--- lock succeed for the first initilization\n");
              for (i = 0; i < (SHARE_SIZE); i++){
                 PTR[i] = 0;
              //   printf("ok?%d\n");
              }

              printf("the %d size is %d now. \n",ff,PTR[SWAP_SIZE]);
              PTR[SWAP_SIZE]++;
              id = PTR[SWAP_SIZE];
              printf("the size is %d now. \n",PTR[SWAP_SIZE]);
               printf("Memory size: 0x%x\n", SWAP_SIZE);
               while(PTR[SWAP_SIZE]!=2){
                    sleep(1);
                    printf("wait for another program\n");
               }
       }
       else{
            printf("the size is %d now. \n",PTR[SWAP_SIZE]);
            PTR[SWAP_SIZE]++;
            id = PTR[SWAP_SIZE];   
            printf("The total user is at most %d. \n",PTR[SWAP_SIZE]);
            assert(PTR[SWAP_SIZE]==2);
            printf("-------two users both connect to the sharemap------ \n");

            unlock();   
       }

*/
/*
              printf("the size is %d now. \n",ptr[SWAP_SIZE]);
              ptr[SHARE_SIZE]++;
              id = ptr[SHARE_SIZE];
              printf("the size is %d now. \n",ptr[SHARE_SIZE]);
               printf("Memory size: 0x%x\n", TOTAL_SHARE_SIZE);
       }
       else{
            printf("the size is %d now. \n",ptr[SHARE_SIZE]);
            ptr[SHARE_SIZE]++;
            id = ptr[SHARE_SIZE];   
            printf("The total user is at most %d. \n",ptr[SHARE_SIZE]);
            assert(ptr[SHARE_SIZE]==2);
            unlock();   
       }
    
    id = ptr[SHARE_SIZE;    
*/  
 /*         if (close(result->fd) == -1) {
            printf("close error: %s\n", strerror(errno));
            exit(exit_failure);
        }
*///    int j=0;
    result->f_cache_set_t = init_cache_mem();
    return result;
}

mem_t init_mem(int len, bool_t is_phy_mem)
{

    mem_t result = (mem_t) malloc(sizeof(mem_rec));
    len = ((len+BPL-1)/BPL)*BPL;
    result->len = len;
    result->contents = (byte_t *) calloc(len, 1);
//    result->m_file_mem = init_file_mem();
//!!! change for null later;
    printf("debug: init_mem:is_phy_mem? %d\n",is_phy_mem);
    if(!is_phy_mem){
        result-> m_file_mem_t = 0;
    }
    else{

         result->m_file_mem_t = init_file_mem(); 

         //???
/*      int a= fork();
        int usernum = 0;
        if(a==0){
            ff = 0;
            usernum = ++result->m_file_mem_t->f_share_mem_t[SWAP_SIZE];
        }
        if(a!=0){
            ff = 1;
            while(result->m_file_mem_t->f_share_mem_t[SWAP_SIZE]!=2)
                sleep(1);
                printf("wait for the child\n");
        }
    
    printf("now the total user is%d\n",usernum);    
*/  }
    printf("initialize finished\n");
    return result;
}
/*
mem_t old_init_mem(int len, bool_t is_phy_mem)
{
    mem_t result = (mem_t)malloc(sizeof(mem_rec));
    len = ((len+BPL-1)/BPL)*BPL;
    result->len = len;
}
*/
void clear_mem(mem_t m)
{
        printf("clear_mem\n");
    memset(m->contents, 0, m->len);

 //   memset(m->m_cache_set_t->cache_matrix,0,sizeof(m->m_cache_set_t->cache_matrix));
 //   void *str = (byte_t*)calloc(SHARE_SIZE,sizeof(byte_t));
   /* if (write(m->m_file_mem_t->fd, str, sizeof(int)) == -1) {
            printf("write error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
    }  
*/
}   

void free_mem(mem_t m)
{
    printf("free_mem\n");
    free((void *) m->contents);
    if(m->m_file_mem_t){
//  free((void *) m->m_file_mem_t -> f_share_mem_t);
//  free((void *) m->m_file_mem_t -> f_cache_set_t);
//  free((void *) m->m_cache_set_t->cache_matrix);
//  free((void *) m->m_cache_set_t);
//  free((void *) m->m_file_mem_t);
    if(munmap(m->m_file_mem_t->fd,SHARE_SIZE)==-1){
        printf("munmap error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
    }
    if(m->m_file_mem_t -> f_share_mem_t)
        free((void *) m->m_file_mem_t -> f_share_mem_t);
        else 
        printf("why null...m->m_file_mem_t -> f_share_mem_t\n");
    
    if(m->m_file_mem_t -> f_cache_set_t)
        free((void *) m->m_file_mem_t -> f_cache_set_t);
    else
        printf("why null...m->m_file_mem_t -> f_cache_set_t\n");
    free((void *) m->m_file_mem_t);
    }
    free((void *) m);
    printf("free_mem\n");
}

mem_t copy_mem(mem_t oldm)
{
    mem_t newm =init_mem(oldm->len,0);
    printf("copy_init %d\n",!(!oldm->m_file_mem_t));
    memcpy(newm->contents, oldm->contents, oldm->len);
//    if(oldm->m_file_mem_t !=0)
//      memcpy(newm->m_file_mem_t,oldm->m_file_mem_t,SHARE_SIZE); 
//    printf("copy_mem\n"); 
    return newm;
}


//!!! YOu need to add the choose ~ wheather the content is in old or new, or you should write another function~~~
bool_t diff_mem(mem_t oldm, mem_t newm, FILE *outfile)
{
    printf("diff_mem\n");
    word_t pos;
    int len = oldm->len;
    bool_t diff = FALSE;
    if (newm->len < len)
    len = newm->len;
    for (pos = 0; (!diff || outfile) && pos < MAX_PRIVATE_MEM; pos += 4) {
        word_t ov = 0;  word_t nv = 0;
    get_word_val(oldm, pos, &ov);
    get_word_val(newm, pos, &nv);
    if (nv != ov) {
        diff = TRUE;
        if (outfile)
        fprintf(outfile, "0x%.4x:\t0x%.8x\t0x%.8x\n", pos, ov, nv);
    }
    }
    return diff;
}


int hex2dig(char c)
{
    if (isdigit((int)c))
    return c - '0';
    if (isupper((int)c))
    return c - 'A' + 10;
    else
    return c - 'a' + 10;
}

#define LINELEN 4096

int load_mem(mem_t m, FILE *infile, int report_error)
{
    /* Read contents of .yo file */
    printf("load_mem\n");
    char buf[LINELEN];
    char c, ch, cl;
    int byte_cnt = 0;
    int lineno = 0;
    word_t bytepos = 0;
    int empty_line = 1;
    int addr = 0;
    char hexcode[15];

#ifdef HAS_GUI
    /* For display */
    int line_no = 0;
    char line[LINELEN];
#endif /* HAS_GUI */   

    int index = 0;

    while (fgets(buf, LINELEN, infile)) {
    int cpos = 0;
    empty_line = 1;
    lineno++;
    /* Skip white space */
    while (isspace((int)buf[cpos]))
        cpos++;

    if (buf[cpos] != '0' ||
        (buf[cpos+1] != 'x' && buf[cpos+1] != 'X'))
        continue; /* Skip this line */      
    cpos+=2;

    /* Get address */
    bytepos = 0;
    while (isxdigit((int)(c=buf[cpos]))) {
        cpos++;
        bytepos = bytepos*16 + hex2dig(c);
    }

    while (isspace((int)buf[cpos]))
        cpos++;

    printf("in the load \n");
    if (buf[cpos++] != ':') {
        if (report_error) {
        fprintf(stderr, "Error reading file. Expected colon\n");
        fprintf(stderr, "Line %d:%s\n", lineno, buf);
        fprintf(stderr, "Reading '%c' at position %d\n", buf[cpos], cpos);
        }
        printf("load_finish\n");
        return 0;
    }

    addr = bytepos;

    while (isspace((int)buf[cpos]))
        cpos++;

    index = 0;

    /* Get code */
    while (isxdigit((int)(ch=buf[cpos++])) && 
           isxdigit((int)(cl=buf[cpos++]))) {
        byte_t byte = 0;
        if (bytepos >= m->len) {
        if (report_error) {
            fprintf(stderr,
                "Error reading file. Invalid address. 0x%x\n",
                bytepos);
            fprintf(stderr, "Line %d:%s\n", lineno, buf);
        }
        printf("load_finish\n");
        return 0;
        }
        byte = hex2dig(ch)*16+hex2dig(cl);
        m->contents[bytepos++] = byte;
        byte_cnt++;
        empty_line = 0;
        hexcode[index++] = ch;
        hexcode[index++] = cl;
    }
    /* Fill rest of hexcode with blanks */
    for (; index < 12; index++)
        hexcode[index] = ' ';
    hexcode[index] = '\0';

#ifdef HAS_GUI
    if (gui_mode) {
        /* Now get the rest of the line */
        while (isspace((int)buf[cpos]))
        cpos++;
        cpos++; /* Skip over '|' */
        
        index = 0;
        while ((c = buf[cpos++]) != '\0' && c != '\n') {
        line[index++] = c;
        }
        line[index] = '\0';
        if (!empty_line)
        report_line(line_no++, addr, hexcode, line);
    }
#endif /* HAS_GUI */ 
    }
            printf("load_finish\n");
    return byte_cnt;
}

// type: 0 for read and 1 for write.
// 0 for unresponse 1 for response.


bool_t send_message(mem_t m, int type,int address){

    printf("%d : send a message to address %.4x, type is %d\n",id, address,type );
    if(address < MAX_PRIVATE_MEM | address > m-> len){
        printf("%d : send_message: position is out of standard range\n",id);
        return FALSE;
    }
    //if the first time... things will be different ...
 /*   while(GETRESPONSE(m)==0){
        printf("I'm prepare to send a new message \n \
            There is a former one come form another program");
        //receive_message(m);
        // a short delay may be needed.
        usleep(100);
    }
*/  GETRESPONSE(m) = 0; //bus[2]=0;
    printf("+++++");
    printf("(make message)%.8x\n",MAKEMESSAGE(id,type,address));
    GETMESSAGE(m)=MAKEMESSAGE(id,type,address);//bus[1] = value;
    printf("getmessage%.8x\n",GETMESSAGE(m));
//    GETSEND(m)=1;
    while(GETTYPE(GETMESSAGE(m))!=BUBBLE){
        printf("(wait response...%d)\n",ff);
        usleep(1000);
/*        if(GETTYPE(m) < 2){
            printf("%d : no body can receive my message\n",id);
            GETRESPONSE(m)=1;

            
        }*/
    }
    usleep(100);
    return TRUE;
}
bool_t receive_message(mem_t m){
//---    printf("(%d,prepare to receive a message)\n",id);
//---    printf("GETSEND: %d\n",GETSEND(m));
    //if some one send a message, I'll know that instantly.
    int message = GETMESSAGE(m);
    int receive_id = GETID(message);
    int type = GETTYPE(message);
    int address = GETADDRESS(message);

    if(receive_id == id){
        printf("(how many people are there)%d\n",GETUSERNUM(m) );
        printf("%d: hit your self\n",id);
        sleep(0.01);
        return FALSE;
    }

    else if(GETTYPE(GETMESSAGE(m))== BUBBLE){
        // do nothing;
        // a short delay may be needed.
//        usleep(100);
//        printf("don't need to response since nobody send a message\n");
        return FALSE;
    }

    else{
        printf("~~~Hitting form the receive_message\n");
        printf("message is %.8x\n",message);
        printf("address is %d\n", address);
        printf("now is %d\n",id );
        cache_line_t clt = hit_cache(m->m_file_mem_t->f_cache_set_t,address);
        if(clt!=NULL){   
            printf("~~~Hitting form the receive_message succeed");
            printf("anything wrong?%d \n",type);
            printf("hit,it %d\n",clt->dirt);
            if(type == READ){
            //read
            printf("read it%d \n",clt->dirt);
            if(clt->dirt){
                printf("write_back\n");
                write_back(m,clt,address);
//                clt->dirt = FALSE;
                printf("%d : Asking for position %.0x, write_back\n",id,address);
            }
        } 
            else if(type == WRITE ){
                //write
                clt->flag = 0;
            }
        }
        else
            printf("doen't hit it\n");

        GETRESPONSE(m)=1;
//        GETSEND(m)=1;
        //
        printf("%d response over\n",2);
        return TRUE;
    }
}


/*pos is 32 bit. however, the max address is 2^16.
which means only the left 16 bit is necessary. 
typedef struct {
  bool_t flag;
// last 10 bytes for tags.
  int tags;
// eight blocks
  byte_t contents[8];
}cache_line, *cache_line_t;

typedef struct {
 cache_line cache_matrix[8][8];
 file_mem_t c_file_mem_t;
}cache_set, cache_set_t;

*/

cache_line_t hit_cache(cache_set_t cst ,word_t pos){
//    cache_line_t = cache_set_t[GET_SETS(pos)]; 

        printf("need hit!, position is %.0x, set is %d\n", pos,GET_SETS(pos));
        bool_t hit= FALSE;
        int set = GET_SETS(pos);
        int i;
        for (i=0;i<8;i++){
            cache_line_t clt_temp= &(cst->cache_matrix[set][i]);
            printf("i want to see the small flags %d\n !",clt_temp->flag);
            if(clt_temp->flag){
                printf("hit cache flag:the flage is %d\n",clt_temp->flag);
                if(GET_ATAGS(pos)==GET_CTAGS(clt_temp->tags))
                {
                    hit = TRUE;
                    int i;
/*          for(i=0;i<8;i++){
             printf("cl_temp %d : %d\n",i,cl_temp.contents[i]);
             (*clt)->contents[i] = cl_temp.contents[i];
           }
            (*clt)->flag = cl_temp.flag;
            (*clt)->dirt = FALSE;
*/                  printf("hit cache: clt->flage,clt%d,%d\n",(clt_temp)->flag,clt_temp);
                    printf("hit success: word postion:%d \
                    set:%d \n", pos,set);
                    return clt_temp;
                    break;
                }
            }
        }
        return 0;
}

bool_t write_byte_back(mem_t m, cache_line_t clt, word_t addr){

}

bool_t write_back(mem_t m,cache_line_t clt,word_t addr){
        if(!clt->dirt){
            clt->flag = FALSE;
        }else{
            printf("write back at pos %.0x\n", addr); // debug
            printf("contents are:");
            int i;
            for (i = 0; i < 8; i++)
            printf("%x", clt->contents[i]);
            printf("\n");
        }

        if(addr < MAX_PRIVATE_MEM)
        {
            printf("Attention:write a share part into the private part!");  
            return FALSE;
        }
    //!!! sth different.. physical contents...
        printf("seg?\n");
        printf("the Real physical address we are going to write-back is %x\n",m->m_file_mem_t->f_share_mem_t+addr);
//        lock(m);
        /*int* newcontent[2];
        int m1 =((int*)(byte_t*)m->m_file_mem_t->f_share_mem_t+addr-(addr & (0x0F))-MAX_PRIVATE_MEM)[0];
        int m2 =((int*)(byte_t*)m->m_file_mem_t->f_share_mem_t+addr-(addr & (0x0F))-MAX_PRIVATE_MEM)[1];
        int c1 = ((int*)(byte_t*)contents)[0];
        int c2 = ((int*)(byte_t*)contents)[1];
        newcontent[0]=m1&*/
        memcpy((byte_t *)m->m_file_mem_t->f_share_mem_t+addr-(addr & (ALGIN))-(byte_t *)MAX_PRIVATE_MEM,clt->contents,8);
        byte_t *shared;
        int fd;
        if ((fd = open(SHARE_FILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
            printf("open error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (( shared = mmap(NULL, SHARE_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED| MAP_FILE, fd, 0)) == MAP_FAILED) {
            printf("open error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        close(fd);
        printf("check %0.4x,%0.8x\n",addr-(addr & (ALGIN))-MAX_PRIVATE_MEM,(int*)(byte_t *)shared[addr-(addr & (ALGIN))-MAX_PRIVATE_MEM]);
       /* if(munmap(fd,SHARE_SIZE)==-1){
            printf("munmap error: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }*/
//        unlock();
        clt->dirt = FALSE;
        return TRUE;
}

cache_line_t recharge_cache(mem_t m, word_t pos){
    send_message(m,READ,pos);
    cache_set_t cst = m->m_file_mem_t->f_cache_set_t;

    int set = GET_SETS(pos);
    printf("recharge:%d\n",set);
    cache_line_t clt=0;
    bool_t get=FALSE;
    //step 1 : choose a place to write on.
    int i;
    for(i=0;i<8;i++){
        if(!cst->cache_matrix[set][i].flag){
            clt = &cst->cache_matrix[set][i]; 
            get = TRUE;
            break;
        }
    }
    if(!get){
        printf("all position are valid \n");
        for(i=0;i<8;i++)
            if(!cst->cache_matrix[set][i].dirt){
                clt = &cst->cache_matrix[set][i]; 
                get = TRUE;
                break;
            }
    }
    //step 2: if there's no place... choose a dirty on and write-back it
    if(!get){
        printf("all position are dirty\n");
        clt = &cst->cache_matrix[set][rand()%8];
        word_t addr = GET_ADDR(clt,set);
        printf("recharge_cache_step2: the address is %d \n",addr);
        write_back(m,clt,addr);
    }

    //step 3: reload the things from the meomory
    //!!! don't know why
    //printf("recharge_cache_step3: the address is %d \n",addr);
//    memcpy(clt->contents,VALID_ADDR(m->m_file_mem_t->f_share_mem_t+addr),8);
//    printf("really valided ? %d\n",m->m_file_mem_t[(VALID_ADDR(pos)-SHARE_SIZE)/4]);
    printf("recharge:the position is %.8x\n",(pos) - (pos & ALGIN) + m->m_file_mem_t->f_share_mem_t - MAX_PRIVATE_MEM);
    memcpy(clt->contents, (pos)+(m->m_file_mem_t->f_share_mem_t - MAX_PRIVATE_MEM)-((pos)&ALGIN),8);
    clt->flag = TRUE;
    clt->dirt = FALSE; // the dirt is false, since you copy the content from the memeory.
    clt->tags = GET_ATAGS(pos);

    // debug
    printf("load cache, contents are\n");
    for (i = 0; i < 8; i++)
        printf("%x", clt->contents[i]);
    printf("\n"); 

    return clt;
}


bool_t get_byte_val_file(mem_t m, word_t pos, byte_t *dest)
{
        printf("%d: get_byte_val%.4x\n", id,pos);
    if(pos<0)
       return FALSE;
    if(pos+4 < MAX_PRIVATE_MEM){
       printf("share part visit the private part\n");
       return FALSE;
    }
    else if(pos+4 <= MEM_SIZE){
    assert(m->m_file_mem_t!=0);
        cache_set_t cst = m->m_file_mem_t->f_cache_set_t;
//        cache_line_t clt = NULL;
    printf("get ready for hit!\n");

    printf("~~~Hitting from get_byte_val\n");
        cache_line_t clt= hit_cache(cst,pos);
        if(!clt){
//            lock(m);
            printf("~~~Hitting from get_byte_val failed\n" );
            clt = (recharge_cache(m,pos));
//                unlock(m);
        }
            printf("~~~Hitting from get_byte_val succeed\n" );

        *dest = clt->contents[(pos)&(0x7)];
        //*dest = clt->contents + VALID_ADDR(pos);
        return TRUE;
    }
    else{
        printf("get_byte_val_file:%d, out of range\n", pos);
        return FALSE;
    }
}

bool_t get_byte_val(mem_t m, word_t pos, byte_t *dest)
{
//    if (pos < 0 || pos >= m->len)
//  return FALSE;
    if(pos<0)
       return FALSE;
    else if(pos < MAX_PRIVATE_MEM){
        printf("%d,get_byte_val:%.4x,Visit the private part\n",id,pos);
        *dest = m->contents[pos];
        return TRUE;
    }
    else if(pos < m->len){
        printf("%d :get_byte_val_file:%d,Visit the share part,lockget\n",id, pos);
        lock(m);
        if(get_byte_val_file(m,pos,dest)){
            unlock();
            return TRUE;
        }
        unlock();
    }
    else {
        printf("get_byte_val:%d, the address is out of the largest memory part\n", pos);
        return FALSE;
    }
}

    //things we need are not in the cache...
    //Reading from the physical memory is necessory.....
bool_t get_word_val_file(mem_t m,word_t pos, word_t * dest){
//--    printf("get_word_val%d\n", pos);
    int i;
    word_t val=0;
    if(pos < 0)
       return FALSE;
    if(pos+4 <= MAX_PRIVATE_MEM){
       printf("share part visit the private part\n");
       return FALSE;
    }
// Add part for deal with the share data.
// In fact the shared data should be put into the memory part...
    else{
       for(i=0;i<4;i++){
            byte_t val_sub = 0; 
            if(get_byte_val_file(m,pos+i,&val_sub)){
                val = val | val_sub << (8*i);
            }
            else
                printf("A misktake happened in get_word_val_file");
       }
            *dest = val;
    return TRUE;
    }
}

bool_t get_word_val(mem_t m, word_t pos, word_t *dest)
{
    int i;
    word_t val;
    if(pos<0)
       return FALSE;
    if(pos + 4 <= MAX_PRIVATE_MEM){
//--        printf("get_word_val:%d,Visit the private part\n",pos);
        val = 0;
        for (i = 0; i < 4; i++)
            val = val | m->contents[pos+i]<<(8*i);
        *dest = val;
        return TRUE;
    }
    else if(pos+4 <= m->len){
    printf("%d: get_word_val:%.4x,Visit the public part lockget_word_val\n",id,pos);
        lock(m);
        if(get_word_val_file(m,pos,dest)){
            unlock();
            return TRUE;
        }
        else{
            unlock();
            return FALSE;
        }
    }
    else{
        printf("get_word_val_file:%d, out of range\n", pos);
        return FALSE;
    }
}





//set part....
bool_t set_byte_val_file(mem_t m,word_t pos, byte_t val)
{
    printf("%d: set_byte_val%.4x\n",id, pos);
    if(pos < 0)
       return FALSE;
    if(pos < MAX_PRIVATE_MEM){
       printf("share part visit the private part\n");
       return FALSE;
    }
    printf("~~~Hitting from_set_byte_val\n");
    cache_line_t clt = hit_cache(m->m_file_mem_t->f_cache_set_t,pos);
    if(!clt){
//        lock(m);
            printf("~~~Hitting from_set_byte_val failed\n");
            printf("set_byte_val_file: didn't hit,need to load new cache!");
            clt = (recharge_cache(m,pos));

 //           unlock(m);
 //       else
 //           return FALSE;
    }
    printf("~~~Hitting from_set_byte_val succeed\n");
    printf("~~~~~~~\n");
    printf("%d:set_byte_val_file: the write position is %d,%d\n",id,pos,(pos)&(0x7));
    printf("contents size%d,%d,%d\n",sizeof(clt->contents),clt->flag,clt); 

// write something into the cache.
    send_message(m,WRITE,pos);
    clt->contents[((pos)&(0x7))] = val; // I only need the last 3.
    clt->dirt = TRUE;
//    clt->flag = TRUE; only one word is set to be zero...
    printf("leave the set_byte_val_file\n");
    return TRUE;
}

bool_t set_byte_val(mem_t m, word_t pos, byte_t val)
{
   if(pos<0)
       return FALSE;
    else if(pos <= MAX_PRIVATE_MEM){
        printf("%d: set_byte_val:%.4x,Visit the private part\n",id,pos);
        m->contents[pos] = val;
        return TRUE;
    }
    else if(pos <= m->len){
        printf("%d: set_byte_val:%.4x,Visit the share part_lockset\n",id,pos);
        lock(m);
        if(set_byte_val_file(m,pos,val)){
            unlock();
            return TRUE;
        }
        else {
            unlock();
            return FALSE;
        }
    }
    else{
        printf("%d: set_byte_val_file:%.4x, out of range\n",id, pos);
        return FALSE;
    }
}



bool_t set_word_val_file(mem_t m,word_t pos, word_t val)
{
    if(pos < 0)
       return FALSE;
    if(pos+4 <= MAX_PRIVATE_MEM){
       printf("share part visit the private part\n");
       return FALSE;
    }
    else{
        int i;
        for (i = 0; i < 4; ++i) {
            if(set_byte_val_file(m, pos + i, val & 0xFF))
                val >>= 8;
            else{
                printf("probem in set_word_val_file:%d \n",i);
                return FALSE;
            }
    }
    }
    return TRUE;
}
bool_t set_word_val(mem_t m, word_t pos, word_t val)
{
    int i;
  //  if (pos < 0 || pos + 4 > m->len)
  //    return FALSE;
    if(pos<0)
       return FALSE;
    else if( pos+ 4 <= MAX_PRIVATE_MEM){
        for (i = 0; i < 4; i++) {
           m->contents[pos+i] = val & 0xFF;
           val >>= 8;
        }
        return TRUE;
    }
    else if (pos+4 <= m->len){
        printf("%d lockset_word_val\n", id);
        lock(m);
        if(set_word_val_file(m,pos,val)){
            unlock();
            return TRUE;
        }
        unlock();
    }        
    else{
        printf("set_word_val:%d, out of range\n", pos);
        return FALSE;
    }
}


/*** finish my cahce part*****************/


void dump_memory(FILE *outfile, mem_t m, word_t pos, int len)
{
    int i, j;
    while (pos % BPL) {
    pos --;
    len ++;
    }

    len = ((len+BPL-1)/BPL)*BPL;

    if (pos+len > m->len)
    len = m->len-pos;

    for (i = 0; i < len; i+=BPL) {
    word_t val = 0;
    fprintf(outfile, "0x%.4x:", pos+i);
    for (j = 0; j < BPL; j+= 4) {
        get_word_val(m, pos+i+j, &val);
        fprintf(outfile, " %.8x", val);
    }
    }
}

mem_t init_reg()
{
    return init_mem(32,0);
}

void free_reg(mem_t r)
{
    free_mem(r);
}

mem_t copy_reg(mem_t oldr)
{
    return copy_mem(oldr);
}

bool_t diff_reg(mem_t oldr, mem_t newr, FILE *outfile)
{
    word_t pos;
    int len = oldr->len;
    bool_t diff = FALSE;
    if (newr->len < len)
    len = newr->len;
    for (pos = 0; (!diff || outfile) && pos < len; pos += 4) {
        word_t ov = 0;
        word_t nv = 0;
    get_word_val(oldr, pos, &ov);
    get_word_val(newr, pos, &nv);
    if (nv != ov) {
        diff = TRUE;
        if (outfile)
        fprintf(outfile, "%s:\t0x%.8x\t0x%.8x\n",
            reg_table[pos/4].name, ov, nv);
    }
    }
    return diff;
}


word_t get_reg_val(mem_t r, reg_id_t id)
{
    printf("get_reg_val%d\n", id);
    word_t val = 0;
    if (id >= REG_NONE)
    return 0;
    get_word_val(r,id*4, &val);
    return val;
}


void set_reg_val(mem_t r, reg_id_t id, word_t val)
{
    if (id < REG_NONE) {
    set_word_val(r,id*4,val);
#ifdef HAS_GUI
    if (gui_mode) {
        signal_register_update(id, val);
    }
#endif /* HAS_GUI */
    }
}
     
void dump_reg(FILE *outfile, mem_t r) {
    reg_id_t id;
    for (id = 0; reg_valid(id); id++) {
    fprintf(outfile, "   %s  ", reg_table[id].name);
    }
    fprintf(outfile, "\n");
    for (id = 0; reg_valid(id); id++) {
    word_t val = 0;
    get_word_val(r, id*4, &val);
    fprintf(outfile, " %x", val);
    }
    fprintf(outfile, "\n");
}

struct {
    char symbol;
    int id;
} alu_table[A_NONE+1] = 
{
    {'+',   A_ADD},
    {'-',   A_SUB},
    {'&',   A_AND},
    {'^',   A_XOR},
    {'?',   A_NONE}
};

char op_name(alu_t op)
{
    if (op < A_NONE)
    return alu_table[op].symbol;
    else
    return alu_table[A_NONE].symbol;
}

word_t compute_alu(alu_t op, word_t argA, word_t argB)
{
    word_t val;
    switch(op) {
    case A_ADD:
    val = argA+argB;
    break;
    case A_SUB:
    val = argB-argA;
    break;
    case A_AND:
    val = argA&argB;
    break;
    case A_XOR:
    val = argA^argB;
    break;
    default:
    val = 0;
    }
    return val;
}

cc_t compute_cc(alu_t op, word_t argA, word_t argB)
{
    word_t val = compute_alu(op, argA, argB);
    bool_t zero = (val == 0);
    bool_t sign = ((int)val < 0);
    bool_t ovf;
    switch(op) {
    case A_ADD:
        ovf = (((int) argA < 0) == ((int) argB < 0)) &&
           (((int) val < 0) != ((int) argA < 0));
    break;
    case A_SUB:
        ovf = (((int) argA > 0) == ((int) argB < 0)) &&
           (((int) val < 0) != ((int) argB < 0));
    break;
    case A_AND:
    case A_XOR:
    ovf = FALSE;
    break;
    default:
    ovf = FALSE;
    }
    return PACK_CC(zero,sign,ovf);
    
}

char *cc_names[8] = {
    "Z=0 S=0 O=0",
    "Z=0 S=0 O=1",
    "Z=0 S=1 O=0",
    "Z=0 S=1 O=1",
    "Z=1 S=0 O=0",
    "Z=1 S=0 O=1",
    "Z=1 S=1 O=0",
    "Z=1 S=1 O=1"};

char *cc_name(cc_t c)
{
    int ci = c;
    if (ci < 0 || ci > 7)
    return "???????????";
    else
    return cc_names[c];
}

/* Status types */

char *stat_names[] = { "BUB", "AOK", "HLT", "ADR", "INS", "PIP" };

char *stat_name(stat_t e)
{
    if (e < 0 || e > STAT_PIP)
    return "Invalid Status";
    return stat_names[e];
}

/**************** Implementation of ISA model ************************/

state_ptr new_state(int memlen)
{
    state_ptr result = (state_ptr) malloc(sizeof(state_rec));
    result->pc = 0;
    result->r = init_reg();
    result->m = init_mem(memlen,1);
    result->cc = DEFAULT_CC;
    return result;
}

void free_state(state_ptr s)
{
    free_reg(s->r);
    free_mem(s->m);
    free((void *) s);
}

state_ptr copy_state(state_ptr s) {
    state_ptr result = (state_ptr) malloc(sizeof(state_rec));
    result->pc = s->pc;
    result->r = copy_reg(s->r);
    result->m = copy_mem(s->m);
    result->cc = s->cc;
    printf("copy_state\n");
    return result;
}

bool_t diff_state(state_ptr olds, state_ptr news, FILE *outfile) {
    bool_t diff = FALSE;

    if (olds->pc != news->pc) {
    diff = TRUE;
    if (outfile) {
        fprintf(outfile, "pc:\t0x%.8x\t0x%.8x\n", olds->pc, news->pc);
    }
    }
    if (olds->cc != news->cc) {
    diff = TRUE;
    if (outfile) {
        fprintf(outfile, "cc:\t%s\t%s\n", cc_name(olds->cc), cc_name(news->cc));
    }
    }
    if (diff_reg(olds->r, news->r, outfile))
    diff = TRUE;
    if (diff_mem(olds->m, news->m, outfile))
    diff = TRUE;
    return diff;
}


/* Branch logic */
bool_t cond_holds(cc_t cc, cond_t bcond) {
    bool_t zf = GET_ZF(cc);
    bool_t sf = GET_SF(cc);
    bool_t of = GET_OF(cc);
    bool_t jump = FALSE;
    
    switch(bcond) {
    case C_YES:
    jump = TRUE;
    break;
    case C_LE:
    jump = (sf^of)|zf;
    break;
    case C_L:
    jump = sf^of;
    break;
    case C_E:
    jump = zf;
    break;
    case C_NE:
    jump = zf^1;
    break;
    case C_GE:
    jump = sf^of^1;
    break;
    case C_G:
    jump = (sf^of^1)&(zf^1);
    break;
    default:
    jump = FALSE;
    break;
    }
    return jump;
}


/* Execute single instruction.  Return status. */
stat_t step_state(state_ptr s, FILE *error_file)
{
    word_t argA, argB;
    byte_t byte0 = 0;
    byte_t byte1 = 0;
    itype_t hi0;
    alu_t  lo0;
    reg_id_t hi1 = REG_NONE;
    reg_id_t lo1 = REG_NONE;
    bool_t ok1 = TRUE;
    word_t cval = 0;
    word_t okc = TRUE;
    word_t val, dval;
    bool_t need_regids;
    bool_t need_imm;
    word_t ftpc = s->pc;  /* Fall-through PC */

    if (!get_byte_val(s->m, ftpc, &byte0)) {
    if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
    return STAT_ADR;
    }
    ftpc++;

    hi0 = HI4(byte0);
    lo0 = LO4(byte0);

    need_regids =
    (hi0 == I_RRMOVL || hi0 == I_ALU || hi0 == I_PUSHL ||
     hi0 == I_POPL || hi0 == I_IRMOVL || hi0 == I_RMMOVL ||
     hi0 == I_MRMOVL || hi0 == I_IADDL);

    if (need_regids) {
    ok1 = get_byte_val(s->m, ftpc, &byte1);
    ftpc++;
    hi1 = HI4(byte1);
    lo1 = LO4(byte1);
    }

    need_imm =
    (hi0 == I_IRMOVL || hi0 == I_RMMOVL || hi0 == I_MRMOVL ||
     hi0 == I_JMP || hi0 == I_CALL || hi0 == I_IADDL);

    if (need_imm) {
    okc = get_word_val(s->m, ftpc, &cval);
    ftpc += 4;
    }

    switch (hi0) {
    case I_NOP:
    s->pc = ftpc;
    break;
    case I_HALT:
    return STAT_HLT;
    break;
    case I_RRMOVL:  /* Both unconditional and conditional moves */
    if (!ok1) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    if (!reg_valid(hi1)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid register ID 0x%.1x\n",
            s->pc, hi1);
        return STAT_INS;
    }
    if (!reg_valid(lo1)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid register ID 0x%.1x\n",
            s->pc, lo1);
        return STAT_INS;
    }
    val = get_reg_val(s->r, hi1);
    if (cond_holds(s->cc, lo0))
      set_reg_val(s->r, lo1, val);
    s->pc = ftpc;
    break;
    case I_IRMOVL:
    if (!ok1) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    if (!okc) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address",
            s->pc);
        return STAT_INS;
    }
    if (!reg_valid(lo1)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid register ID 0x%.1x\n",
            s->pc, lo1);
        return STAT_INS;
    }
    set_reg_val(s->r, lo1, cval);
    s->pc = ftpc;
    break;
    case I_RMMOVL:
    if (!ok1) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    if (!okc) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_INS;
    }
    if (!reg_valid(hi1)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid register ID 0x%.1x\n",
            s->pc, hi1);
        return STAT_INS;
    }
    if (reg_valid(lo1)) 
        cval += get_reg_val(s->r, lo1);
    val = get_reg_val(s->r, hi1);
    if (!set_word_val(s->m, cval, val)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid data address 0x%x\n",
            s->pc, cval);
        return STAT_ADR;
    }
    s->pc = ftpc;
    break;
    case I_MRMOVL:
    if (!ok1) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    if (!okc) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction addres\n", s->pc);
        return STAT_INS;
    }
    if (!reg_valid(hi1)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid register ID 0x%.1x\n",
            s->pc, hi1);
        return STAT_INS;
    }
    if (reg_valid(lo1)) 
        cval += get_reg_val(s->r, lo1);
    if (!get_word_val(s->m, cval, &val))
        return STAT_ADR;
    set_reg_val(s->r, hi1, val);
    s->pc = ftpc;
    break;
    case I_ALU:
    if (!ok1) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    argA = get_reg_val(s->r, hi1);
    argB = get_reg_val(s->r, lo1);
    val = compute_alu(lo0, argA, argB);
    set_reg_val(s->r, lo1, val);
    s->cc = compute_cc(lo0, argA, argB);
    s->pc = ftpc;
    break;
    case I_JMP:
    if (!ok1) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    if (!okc) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    if (cond_holds(s->cc, lo0))
        s->pc = cval;
    else
        s->pc = ftpc;
    break;
    case I_CALL:
    if (!ok1) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    if (!okc) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    val = get_reg_val(s->r, REG_ESP) - 4;
    set_reg_val(s->r, REG_ESP, val);
    if (!set_word_val(s->m, val, ftpc)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid stack address 0x%x\n", s->pc, val);
        return STAT_ADR;
    }
    s->pc = cval;
    break;
    case I_RET:
    /* Return Instruction.  Pop address from stack */
    dval = get_reg_val(s->r, REG_ESP);
    if (!get_word_val(s->m, dval, &val)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid stack address 0x%x\n",
            s->pc, dval);
        return STAT_ADR;
    }
    set_reg_val(s->r, REG_ESP, dval + 4);
    s->pc = val;
    break;
    case I_PUSHL:
    if (!ok1) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    if (!reg_valid(hi1)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid register ID 0x%.1x\n", s->pc, hi1);
        return STAT_INS;
    }
    val = get_reg_val(s->r, hi1);
    dval = get_reg_val(s->r, REG_ESP) - 4;
    set_reg_val(s->r, REG_ESP, dval);
    if  (!set_word_val(s->m, dval, val)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid stack address 0x%x\n", s->pc, dval);
        return STAT_ADR;
    }
    s->pc = ftpc;
    break;
    case I_POPL:
    if (!ok1) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    if (!reg_valid(hi1)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid register ID 0x%.1x\n", s->pc, hi1);
        return STAT_INS;
    }
    dval = get_reg_val(s->r, REG_ESP);
    set_reg_val(s->r, REG_ESP, dval+4);
    if (!get_word_val(s->m, dval, &val)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid stack address 0x%x\n",
            s->pc, dval);
        return STAT_ADR;
    }
    set_reg_val(s->r, hi1, val);
    s->pc = ftpc;
    break;
    case I_LEAVE:
    dval = get_reg_val(s->r, REG_EBP);
    set_reg_val(s->r, REG_ESP, dval+4);
    if (!get_word_val(s->m, dval, &val)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid stack address 0x%x\n",
            s->pc, dval);
        return STAT_ADR;
    }
    set_reg_val(s->r, REG_EBP, val);
    s->pc = ftpc;
    break;
    case I_IADDL:
    if (!ok1) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address\n", s->pc);
        return STAT_ADR;
    }
    if (!okc) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction address",
            s->pc);
        return STAT_INS;
    }
    if (!reg_valid(lo1)) {
        if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid register ID 0x%.1x\n",
            s->pc, lo1);
        return STAT_INS;
    }
    argB = get_reg_val(s->r, lo1);
    val = argB + cval;
    set_reg_val(s->r, lo1, val);
    s->cc = compute_cc(A_ADD, cval, argB);
    s->pc = ftpc;
    break;
    default:
    if (error_file)
        fprintf(error_file,
            "PC = 0x%x, Invalid instruction %.2x\n", s->pc, byte0);
    return STAT_INS;
    }
    return STAT_AOK;
}

void show_memory(mem_t m){
    FILE* pFile;  
//    float mybuffer[SHARE_SIZE/4] = {0};  
    if((pFile = fopen("./share_memory.txt" , "wb"))==-1) // 打开文件写操作  
    {
            printf("open error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
    }
//    fprintf(fp, "%d %s", num, name);   
//     
//    fclose(pFile); // 关闭文件   
    byte_t *share = (int*)m->m_file_mem_t->f_share_mem_t;
    int i;
    for(i=0;i<SHARE_SIZE;++i){
        if(share[i]!=0)
            fprintf(pFile,"0x%.4x: 0x%.8x\n",i,share[i]);
    }
    if (fclose(pFile) == -1) {
            printf("close error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
}




void write_all_back(mem_t m){
    int i,j;
    cache_set_t cst = m->m_file_mem_t->f_cache_set_t;
    for(i = 0;i < 8;i++)
        for(j = 0;j < 8;j++){
            cache_line_t clt = &cst->cache_matrix[i][j];
            if(clt->dirt){
                write_back(m,&cst->cache_matrix[i][j],GET_ADDR(clt,i));
            }
        }
}

bool_t leave(mem_t m){
//    while(GETSEND(m)){
//        receive_message(m);
//    }
    lock(m);
    write_all_back(m);
    unlock();
/*    byte_t *share = m->m_file_mem_t->f_share_mem_t;
    if(GETUSERNUM(m)==0){
        show_memory(m);
        munmap(share,SHARE_SIZE);
    } */
    printf("Goodbye, There left %d users,\n", --GETUSERNUM(m));
    return TRUE;
}
