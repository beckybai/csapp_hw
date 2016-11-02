/* Instruction Set definition for Y86 Architecture */
/* Revisions:
   2009-03-11:
       Changed RNONE to be 0xF
       Changed J_XX and jump_t to C_XX and cond_t; take_branch to cond_holds
       Expanded RRMOVL to include conditional moves
*/

/**************** Registers *************************/

/* REG_NONE is a special one to indicate no register */
typedef enum { REG_EAX, REG_ECX, REG_EDX, REG_EBX,
	       REG_ESP, REG_EBP, REG_ESI, REG_EDI, REG_NONE=0xF, REG_ERR } reg_id_t;

/* Find register ID given its name */
reg_id_t find_register(char *name);
/* Return name of register given its ID */
char *reg_name(reg_id_t id);

/**************** Instruction Encoding **************/

/* Different argument types */
typedef enum { R_ARG, M_ARG, I_ARG, NO_ARG } arg_t;

/* Different instruction types */
typedef enum { I_HALT, I_NOP, I_RRMOVL, I_IRMOVL, I_RMMOVL, I_MRMOVL,
	       I_ALU, I_JMP, I_CALL, I_RET, I_PUSHL, I_POPL,
	       I_IADDL, I_LEAVE, I_POP2,I_RMCHANGE } itype_t;

/* Different ALU operations */
typedef enum { A_ADD, A_SUB, A_AND, A_XOR, A_NONE } alu_t;

/* Default function code */
typedef enum { F_NONE } fun_t;

/* Return name of operation given its ID */
char op_name(alu_t op);

/* Different Jump conditions */
typedef enum { C_YES, C_LE, C_L, C_E, C_NE, C_GE, C_G } cond_t;

/* Pack itype and function into single byte */
#define HPACK(hi,lo) ((((hi)&0xF)<<4)|((lo)&0xF))

/* Unpack byte */
#define HI4(byte) (((byte)>>4)&0xF)
#define LO4(byte) ((byte)&0xF)

/* Get the opcode out of one byte instruction field */
#define GET_ICODE(instr) HI4(instr)

/* Get the ALU/JMP function out of one byte instruction field */
#define GET_FUN(instr) LO4(instr)

/* Return name of instruction given it's byte encoding */
char *iname(int instr);

/**************** Truth Values **************/
typedef enum { FALSE, TRUE } bool_t;

/* Table used to encode information about instructions */
typedef struct {
  char *name;
  unsigned char code; /* Byte code for instruction+op */
  int bytes;
  arg_t arg1;
  int arg1pos;
  int arg1hi;  /* 0/1 for register argument, # bytes for allocation */
  arg_t arg2;
  int arg2pos;
  int arg2hi;  /* 0/1 */
} instr_t, *instr_ptr;

instr_ptr find_instr(char *name);

/* Return invalid instruction for error handling purposes */
instr_ptr bad_instr();

/***********  Things about the broadcast mechanism   ******/
#define GETRESPONSE(m) (((int*)((byte_t*)(m->m_file_mem_t->f_share_mem_t)+(SWAP_SIZE)))[2])
#define GETUSERNUM(m)  (((int*)((byte_t*)(m->m_file_mem_t->f_share_mem_t)+(SWAP_SIZE)))[0])
#define GETMESSAGE(m)  (((int*)((byte_t*)(m->m_file_mem_t->f_share_mem_t)+(SWAP_SIZE)))[1])
//#define GETSEND(m)     (((int*)(byte_t*)(m->m_file_mem_t->f_share_mem_t+(SWAP_SIZE)))[4])

#define MAKEMESSAGE(id,type,address) (((id) << 24) | ((type) << 16) | (address))
#define GETID(me)((me) >> 24)
#define GETTYPE(me)(((me) >> 16) & (0xFF))
#define GETADDRESS(me)((me) & (0xFFFF))
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IWGRP |S_IRGRP | S_IROTH)

/***********  Implementation of Memory *****************/
typedef unsigned char byte_t;
typedef int word_t;

typedef struct {
  bool_t flag;
  bool_t dirt;
// last 10 bytes for tags.
  int tags;
// eight blocks
  byte_t contents[8];
}cache_line, *cache_line_t;



typedef struct {
 cache_line cache_matrix[8][8];
}cache_set, *cache_set_t;


typedef struct {
  int fd;
  byte_t *f_share_mem_t;
  cache_set_t f_cache_set_t;
} file_mem, *file_mem_t;

typedef struct {
  int len;
  word_t maxaddr;
  byte_t *contents;
  file_mem_t m_file_mem_t;
} mem_rec, *mem_t;

/* Represent a memory as an array of bytes */

/* Create a memory with len bytes */
mem_t init_mem(int len,bool_t is_phy_mem);
void free_mem(mem_t m);

/* Set contents of memory to 0 */
void clear_mem(mem_t m);

/* Make a copy of a memory */
mem_t copy_mem(mem_t oldm);
/* Print the differences between two memories */
bool_t diff_mem(mem_t oldm, mem_t newm, FILE *outfile);

/* How big should the memory be? */
#ifdef BIG_MEM
#define MEM_SIZE (1<<16)
#else
#define MEM_SIZE (1<<13)
#endif
#define MAX_PRIVATE_MEM ((MEM_SIZE) >> 1)
#define SHARE_SIZE ((MEM_SIZE)>>1)
#define SWAP_SIZE (((MEM_SIZE)>>3) + ((MEM_SIZE)>>2))
//#define TOTAL_SHARE_SIZE (SHARE_SIZE+SWAP_SIZE)


#define SHARE_FILE "/tmp/share_file.txt"
#define LOCK_FILE "/tmp/y86-lock-file1.txt"

//*******************added instruction
//#define VALID_POS(addr) ((addr) & 0xFFFF)

//#GET ADDRE
#define GET_ATAGS(addr) ((addr)>>6 & 0x3FF) // 11 1111 1111
#define GET_SETS(addr) ((addr)>>3 & 0x7) // 111
#define GET_BLOCK(addr) (((addr)>>2 & 1)*4+((addr)>>1 & 1)*2 + (addr) & 0x1)
#define GET_CTAGS(addr) (addr & 0x3FF) // 11 1111 1111
#define GET_ADDR(cls,set) ((GET_CTAGS(cls->tags)<<6 | set<<3) ) //for align

//#define VALID_ADDR(addr) (addr & 0xF0)/
//#define ADDR_OFFSET(pos) ((pos) & 0xF)

//#define WRITE_BACK(cls)(cls->flag=0)

//********************end*************/


/*** In the following functions, a return value of 1 means success ***/

/* Load memory from .yo file.  Return number of bytes read */
int load_mem(mem_t m, FILE *infile, int report_error);

/* Get byte from memory */
bool_t get_byte_val(mem_t m, word_t pos, byte_t *dest);

/* Get 4 bytes from memory */
bool_t get_word_val(mem_t m, word_t pos, word_t *dest);

/* Set byte in memory */
bool_t set_byte_val(mem_t m, word_t pos, byte_t val);

/* Set 4 bytes in memory */
bool_t set_word_val(mem_t m, word_t pos, word_t val);

/* Print contents of memory */
void dump_memory(FILE *outfile, mem_t m, word_t pos, int cnt);

/********** Implementation of Register File *************/

mem_t init_reg();
void free_reg();

/* Make a copy of a register file */
mem_t copy_reg(mem_t oldr);
/* Print the differences between two register files */
bool_t diff_reg(mem_t oldr, mem_t newr, FILE *outfile);


word_t get_reg_val(mem_t r, reg_id_t id);
void set_reg_val(mem_t r, reg_id_t id, word_t val);
void dump_reg(FILE *outfile, mem_t r);



/* ****************  ALU Function **********************/

/* Compute ALU operation */
word_t compute_alu(alu_t op, word_t arg1, word_t arg2);

typedef unsigned char cc_t;

#define GET_ZF(cc) (((cc) >> 2)&0x1)
#define GET_SF(cc) (((cc) >> 1)&0x1)
#define GET_OF(cc) (((cc) >> 0)&0x1)

#define PACK_CC(z,s,o) (((z)<<2)|((s)<<1)|((o)<<0))

#define DEFAULT_CC PACK_CC(1,0,0)

/* Compute condition code.  */
cc_t compute_cc(alu_t op, word_t arg1, word_t arg2);

/* Generated printed form of condition code */
char *cc_name(cc_t c);

/* **************** Status types *******************/

typedef enum 
 {STAT_BUB, STAT_AOK, STAT_HLT, STAT_ADR, STAT_INS, STAT_PIP } stat_t;

 typedef enum
  { BUBBLE, READ, WRITE } BROADCAST_TYPE;

/* Describe Status */
char *stat_name(stat_t e);

/* **************** ISA level implementation *********/

typedef struct {
  word_t pc;
  mem_t r;
  mem_t m;
  cc_t cc;
} state_rec, *state_ptr;

state_ptr new_state(int memlen);
void free_state(state_ptr s);

state_ptr copy_state(state_ptr s);
bool_t diff_state(state_ptr olds, state_ptr news, FILE *outfile);

/* Determine if condition satisified */
bool_t cond_holds(cc_t cc, cond_t bcond);

/* Execute single instruction.  Return status. */
stat_t step_state(state_ptr s, FILE *error_file);

/************************ Interface Functions *************/

#ifdef HAS_GUI
void report_line(int line_no, int addr, char *hexcode, char *line);
void signal_register_update(reg_id_t r, int val);

#endif

int id;
int ff;
int fc;
int fd;// common file's head
//int fd_lock;// lock file's head/
bool_t receive_message(mem_t m);
bool_t send_message(mem_t m, int type,int address);
bool_t write_back(mem_t m,cache_line_t clt, word_t addr);
cache_line_t hit_cache(cache_set_t cst ,word_t pos);
bool_t leave(mem_t m);
void show_memory(mem_t m);
void show_share_memory();