

#define ERR_OPEN_READ       "could not open '%s' for reading."
#define ERR_OPEN_WRITE      "could not open '%s' for writing."
#define ERR_LINE_TOO_LONG   "source line too long (max is %d)"
#define ERR_NO_ORIG         "no .ORIG directive found"
#define ERR_NO_END          "no .END directive found"
#define ERR_ORIG_NOT_1ST    "instruction(s) appear before .ORIG"
#define ERR_END_NOT_LAST    "instruction(s) appear after .END"
#define ERR_MISSING_ORIG    ".ORIG not found"
#define ERR_MISSING_END     ".END not found"
#define ERR_EXPECTED_COMMA  "expected comma, got '%s'"
#define ERR_EXPECTED_REG    "expected register (R0-R7), got '%s'"
#define ERR_EXPECT_REG_IMM  "expected register or immediate, got '%s'"
#define ERR_EXPECT_lable_IMM  "expected register or immediate, got '%s'"
#define ERR_BAD_LABEL       "label '%s' conflicts "
#define ERR_MISSING_OP      "expected LC3 op, got '%s'"
#define ERR_MISSING_OPERAND "too few operand(s)"
#define ERR_EXTRA_OPERAND   "extra operand(s) '%s'"
#define ERR_DUPLICATE_LABEL "label '%s' previosly defined"
#define ERR_MISSING_LABEL   "label '%s' never defined"
#define ERR_BAD_PCOFFSET    "PCoffset to '%s' out of range"
#define ERR_BAD_IMM         "immediate '%s' (bad format)"
#define ERR_IMM_TOO_BIG     "immediate '%s' out of range"
#define ERR_EXPECTED_STR    "expected quoted string, got '%s'"
#define ERR_ILLEAGLE_INST   "illeagle instruction"
#define ERR_BAD_STR         "unterminated string '%s'"
#define ERR_WRONG_FORMAT    "instruction . parameter doesn't match "
#define ERR_EXPECTED_quot   "expect quot to be a string "
/** LC3 words are 16 bits */
#define LC3_WORD unsigned short

/** The LC3 defines a memory accessed by a 16 bit address */
#define LC3_MEM_SIZE 65536

/** The LC3 contains 8 general purpose register, named R0..R7 */
#define LC3_NUM_REGS 8  

/** Return address stored in R7 */
#define RETURN_ADDR_REG   7

#define SYMBOL_SIZE 997  /**<  997 prime number, the size of hash table  */

/** Maximum length of source line */
#define MAX_LINE_LENGTH 8180

/** Max token in LC3 line, plus a few more to handle bad syntax */
#define MAX_TOKENS 10


 int srcLineNum;

/** A global variable defining the LC3 address of the current instruction */
 int currAddr;

/** A global variable defining the number of errors found */
 int numErrors;

//句子信息 。
typedef struct line_sprt 
{   
    char list[10][50];
    int part_num;
} line_sprt;

typedef enum opcode {
  OP_INVALID = -1, /**< invalid opcode                                       */
  OP_BR,           /**< PC = PCi + PCoffset9 if condition is met             */
  OP_BRN,
  OP_BRZ,
  OP_BRP,
  OP_BRNZ,
  OP_BRNP,
  OP_BRZP,
  OP_BRNZP,
  OP_ADD,          /**< DR = SR1 + SR2 or DR = SR1 + imm5                    */
  OP_LD,           /**< DR = mem[PCi + PCoffset9]                            */
  OP_ST,           /**< mem[PCi + PCoffset9] = SR                            */
  OP_JSR,     /**< R7 = PCi and (PC = SR or PC = PCi + PCoffest9)       */
  OP_JSRR,
  OP_AND,          /**< DR = SR1 & SR2                                       */
  OP_LDR,          /**< DR = mem[BaseR + offset6]                            */
  OP_STR,          /**< mem[BaseR + offset6] = SR                            */
  OP_RTI,          /**< PC = R7, exit supervisor mode                        */
  OP_NOT,          /**< DR = ~SR1                                            */
  OP_LDI,          /**< DR = mem[mem[PCi + PCoffset9]]                       */
  OP_STI,          /**< mem[mem[PCi + offset9]] = SR                         */
  OP_JMP_RET,      /**< PC = R7 (RET) or PC = Rx (JMP Rx)                    */
  OP_RESERVED,     /**< Currently not used                                   */
  OP_LEA,          /**< DR = PCi + PCoffset9                                 */
  OP_TRAP,         /**< R7 = PCi, PC = mem[mem[trap]], enter supervisor mode */
  OP_ORIG,         /**< memory location where code is loaded                 */
  OP_END,          /**< end of propgram - only comments may follow           */
  OP_BLKW,         /**< allocate N words of storage initialized with 0       */
  OP_FILL,         /**< allocate 1 word of storage initialed with operand    */
  OP_STRINGZ,      /**< allocate N+1 words of storage initialized with
                            string and null terminator (1 char per word)     */
  OP_GETC,         /**< Read character from keyboard, no echo      (trap x20)*/
  OP_OUT,          /**< Write one character                        (trap x21)*/
  OP_PUTS,         /**< Write null terminated string               (trap x22)*/
  OP_IN,           /**< Print prompt and read/echo character       (trap x23)*/
  OP_HALT,         /**< Halt execution                             (trap x25)*/
  OP_GETS,         /**< Read string from keyboard, store in memory (trap x26)*/                          
  NUM_OPCODES      /**< Initialized by compiler                              */
} opcode_t;

typedef enum operand {
    FMT_R1    = 0x001, /**< DR or SR                      */
    FMT_R2    = 0x002, /**< SR1 or BaseR                  */
    FMT_R3    = 0x004, /**< SR2                           */
    FMT_CC    = 0x008, /**< condition codes               */
    FMT_IMM5  = 0x010, /**< imm5                          */
    FMT_IMM6  = 0x020, /**< offset6                       */
    FMT_VEC8  = 0x040, /**< trapvect8                     */
    FMT_ASC8  = 0x080, /**< 8-bit ASCII                   */
    FMT_PCO9  = 0x100, /**< label (or address for imm9)   */
    FMT_PCO11 = 0x200, /**< label (or address for imm11)  */
    FMT_IMM16 = 0x400, /**< 16 bits for .FILL. or .ORIG   */
    FMT_STR   = 0x800  /**< operand is string literal     */
} operand_t;

typedef enum operands {
  FMT_      = 0,
  FMT_RRR   = (FMT_R1 | FMT_R2 | FMT_R3),
  FMT_RRI5  = (FMT_R1 | FMT_R2 | FMT_IMM5),
  FMT_L     = FMT_PCO9,
  FMT_R     = FMT_R2,
  FMT_I11   = FMT_PCO11,
  FMT_RL    = (FMT_R1 | FMT_PCO9),
  FMT_RRI6  = (FMT_R1 | FMT_R2 | FMT_IMM6),
  FMT_RR    = (FMT_R1 | FMT_R2),
  FMT_V     = FMT_VEC8,
  FMT_A     = FMT_ASC8,
  FMT_16    = FMT_IMM16,
  FMT_S     = FMT_STR
} operands_t;

typedef struct line_info line_info_t;
struct line_info {
  line_info_t* next;         /**< Allow for linked list                   */
  line_sprt    token_sprt;
  int          op_begin;
  int          lineNum;      /**< Line number in source code              */
  int          address;      /**< LC3 address of instruction              */
  int          machineCode;  /**< The 16 bit LC3 instruction              */
  opcode_t     opcode;       /**< opcode of instruction                   */
  int          form;         /**< which form of instruction (ADD/ADDI)    */
  int          reg1;         /**< DR or SR, if present                    */
  int          reg2;         /**< SR1 or BaseR, if present                */
  int          reg3;         /**< SR2, if present                         */
  int          immediate;    /**< Immediate value if present              */
  char*        reference;    /**< Label referenced by instruction, if any */
};

typedef struct inst_format {
 char*      name;      /**< human readable name (e.g. "ADD")           */
 operands_t operands;  /**< operands for this operation (e.g. FMT_RRR) */
 int        prototype; /**< bits that are constant in this instruction */
} inst_format_t;

typedef struct LC3_inst {
  int           formBit;    /**< -1 if instruction has only one format,
                            else bit that differentiates format.   */
  inst_format_t forms[2];   /**< syntax for both possible forms         */
} LC3_inst_t;

typedef struct symbol {
    char* name; /**< the name of the symbol */
    int   addr; /**< symbol's address in the LC3 memory */
} symbol_t;

typedef struct sym_table{
    symbol_t symbol;
    struct sym_table* next;
} sym_table_t;
