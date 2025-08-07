import argparse
import re
import sys

from enum import Enum, auto, IntEnum

# --------------------------------
# constants and data classes
# --------------------------------


class States(Enum):
    START_LINE = auto() 
    WHITESPACE = auto()
    START_OF_LABEL = auto() 
    LABEL_FOUND = auto()
    BEGIN_DIRECTIVE = auto() 
    INT_DIRECTIVE = auto()
    BYT_DIRECTIVE = auto()
    BTS_DIRECTIVE = auto()
    STR_DIRECTIVE = auto()
    NUMBER_FOUND = auto()
    DIRECTIVE_FOUND = auto()
    DIRECTIVE_DONE = auto()
    LINE_FINISHED = auto() 
    BEGIN_CODE = auto()
    GATHER_TOKENS = auto()


class Directive(Enum): 
    INT = "INT"
    BYT = "BYT"
    BTS = "BTS"
    STR = "STR"


class Mnemonic(IntEnum):
    JMP = 0x01
    JMR = 0x02
    BNZ = 0x03
    BGT = 0x04
    BLT = 0x05
    BRZ = 0x06
    MOV = 0x07
    MOVI = 0x08
    LDA = 0x09
    STR = 0x0A
    LDR = 0x0B
    STB = 0x0C
    LDB = 0x0D
    ISTR = 0x0E
    ILDR = 0x0F
    ISTB = 0x10
    ILDB = 0x11
    ADD = 0x12
    ADDI = 0x13
    SUB = 0x14
    SUBI = 0x15
    MUL = 0x16
    MULI = 0x17
    DIV = 0x18
    SDIV = 0x19
    DIVI = 0x1A
    AND = 0x1B
    OR = 0x1C 
    CMP = 0x1D
    CMPI = 0x1E
    TRP = 0x1F
    ALCI = 0x20 
    ALLC = 0x21 
    IALLC = 0x22
    PSHR = 0x23
    PSHB = 0x24
    POPR = 0x25
    POPB = 0x26
    CALL = 0x27
    RET = 0x28 


REG_MAP = {name: i for i, name in enumerate([
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "R13",
    "R14",
    "R15",
    "PC",
    "SL",
    "SB",
    "SP",
    "FP",
    "HP" 
])
   
}

VALID_OPS = {
        "JMP",
        "JMR",
        "BNZ",
        "BGT",
        "BLT",
        "BRZ",
        "MOV",
        "MOVI",
        "LDA",
        "STR",
        "LDR",
        "STB",
        "LDB",
        "ISTR",
        "ILDR",
        "ISTB",
        "ILDB",
        "ADD",
        "ADDI",
        "SUB",
        "SUBI",
        "MUL",
        "MULI",
        "DIV",
        "SDIV",
        "DIVI",
        "AND",
        "OR",
        "CMP",
        "CMPI",
        "TRP",
        "ALCI",
        "ALLC",
        "IALLC",
        "PSHR",
        "PSHB",
        "POPR",
        "POPB",
        "CALL",
        "RET"
    }


class CharStream:

    def __init__(self, path: str):
        # open as a normal text file
        self.f = open(path, "r", encoding="utf-8")
        self.line_number = 1

    def next_char(self) -> str:
        """consume and return the next character, or '' on eof."""
        ch = self.f.read(1)
        if ch == "\n":
            self.line_number += 1
        return ch or ""
    
    def next_n(self, n: int=1):
        """kinda unsafe, could hit an end of line, but I'm just being careful with it"""
        s = []
        for _ in range(n):
            s.append(self.next_char().upper())
        return "".join(s)

    def peek(self, n: int=1) -> str:
        """look at the next character without consuming it."""
        # remember where we are
        cur = self.f.tell()
        s = self.f.read(n) or ""  # if eof, returns ""
        # rewind back to original position
        self.f.seek(cur)
        return s

    def eof(self) -> bool:
        """true if we've hit eof (i.e. peek() returns '')."""
        return self.peek() == ""
    
    def get_line_count(self) -> int: 
        return self.line_number

    def close(self) -> None:
        self.f.close()
        
    def expect_end_of_line(self):
        ch = self.peek()
        while ch in (" ", "\t", ";"):
            self.next_char()
            ch = self.peek()
        return
        
    def consume_white(self):
        ch = self.peek()
        while ch in (" ", "\t", "\r"):
            self.next_char()
            ch = self.peek()
        return
    
    def consume_everything_until_eol(self):
        ch = self.peek()
        while ch not in ("\n", ""):
            self.next_char()
            ch = self.peek()
        return        


class Assembler:

    def __init__(self, path):
        self.stream = CharStream(path)
        self.state = States.START_LINE
        self.location_counter = 4  # pointer to end of bytes 
        self.symbol_table = {}
        
        self.data_bytes = bytearray(b'\x00\x00\x00\x00')  # RESERVE FIRST 4 BYTES FOR ENTRY 
        self.fixups = []
        
        self.error = False
        self.data_possible = True  # turned off when first encountering a code portion
        
        self.entries = []
        self.current_op = None
        self.current_operands = []
        
    def run(self) -> int:
        handlers = {
            States.START_LINE: self._start_line,
            States.START_OF_LABEL: self._start_of_label,
            States.LABEL_FOUND: self._label_found,
            States.BEGIN_DIRECTIVE: self._begin_directive,
            States.INT_DIRECTIVE: self._int_directive,
            States.BYT_DIRECTIVE: self._byt_directive,
            States.BTS_DIRECTIVE: self._bts_directive,
            States.STR_DIRECTIVE: self._str_directive,
            States.NUMBER_FOUND: self._number_found,
            States.LINE_FINISHED: self._line_finished,
            States.DIRECTIVE_DONE: self._directive_done,
            States.BEGIN_CODE: self._begin_code,
            States.GATHER_TOKENS: self._gather_tokens
            }
        
        while not self.stream.eof() and not self.error: 
            handlers[self.state]()
        
        if self.error:
            return self.debugDump()
        
        self.stream.close()

        if self.entries:
            entry_addr = self.entries[0] [2]
            self.data_bytes[0:4] = bytes(split_to_bytes(entry_addr, 4))
        
        self._assemble_entries()
        
    # --------------------------------
    # constants / maps /etc
    # --------------------------------

    # --------------------------------
    # core logic
    # --------------------------------
    def _start_line(self): 
        # we are concerned with two types of instructions here. 
        # DIRECTIVE -> [optional_label] <.directive> [optional_value] [optional_comment]
        # CODE      -> [optional_lable] <operator> <operand_list> [optional_comment]
        
        if self.stream.eof():
            return
        # if first character of a line is alphanumeric, then there's a label there. Alphanumeric chars in pos 0 on a line start are label beginnings 
        if self._is_alpha_numeric(): 
            # print(self.stream.peek())             
            self.state = States.START_OF_LABEL
            return          
        
        # consume whitespace if there isn't a left-justified alphanumeric characters, that means this line has no label 
        self.stream.consume_white()         
        
        # newlines and semicolons mean that we're done with the line 
        if self.stream.peek() == "\n" or self.stream.peek() == ";":
            self.state = States.LINE_FINISHED
            return 
        
        # check if we're looking at a directive start (".") if data section isnt allowed, then the input is invalid 
        if self._is_directive_start() and self.data_possible: 
            self.stream.next_char()  # consume the . before we ship this to have the directive handled 
            self.state = States.BEGIN_DIRECTIVE    
            return
            
        # we check before this to see if data is possible, so if its not and we still have a ".", then we know its an invalid instruction
        elif self._is_directive_start():
            self._print_error() 
            return
              
        # alphanumeric after code means that we've entered the code section 
        if self._is_alpha_numeric(): 
            if self.data_possible: 
                self.data_possible = False
            
            self.state = States.BEGIN_CODE
            # self._print_error()                 
            return        
        
        # anything else is an invalid instruction
        
        # self._print_error()          
    
    def _begin_code(self) -> None: 
        # Below is valid instruction format: 
        # [optional_label] <operator> <operand_list> [optional_comment]
        self.stream.consume_white()
        op_characters = []
        
        while True:
            nxt = self.stream.peek()
            if nxt.isspace():
                break
            op_characters.append(self.stream.next_char())
            
        op = "".join(op_characters).upper()
        
        if op not in VALID_OPS:
            return self._print_error()
        
        self.current_op = op
        self.current_operands = []  # this is shared by everything, if we're doing a new instruction we have to nuke it 
        self.stream.consume_white()
        
        addr = self.location_counter
        self.entries.append(
            (op, self.current_operands, addr)
        )
        self.data_bytes.extend(b'\x00' * 8)
        self.location_counter += 8  # reserving space for the future 
        self.state = States.GATHER_TOKENS
        
        return

    def _gather_tokens(self):
        # gameplan: consume until you hit a semicolon or newline character. 
        # operands can be the following:
            # Registers: (indicated by 2 or 3 character register names listed in the register table)
            # numeric constants: (represented as literal values)
            # labels: as defined in the assembly file, and later resolved to addresses by the assembler. 
                # LABELS ARE CASE SENSITIVE
        
        while True:
            self.stream.consume_white()
            ch = self.stream.peek()
            
            if ch in (";", "\n", ""):
                break
            
            if ch == "'":  # handling character literals 
                literal = [self.stream.next_char()]  # eat the opening quote 
                ch = self.stream.next_char()
                if ch == "\\":
                    literal.append(ch)
                    literal.append(self.stream.next_char())
                else:
                    literal.append(ch)
                
                closing = self.stream.next_char()
                if closing != "'":  # we have to see the closing quote to make sure its valid syntax 
                    return self._print_error()
                literal.append(closing)
                self.current_operands.append("".join(literal))
                continue
            
            sb = []  # stringbuilder stand-in, reset before looking at the next token
            while ch not in (" ", "\t", ";", "\n", ""):
                sb.append(self.stream.next_char())
                ch = self.stream.peek()
            
            self.current_operands.append("".join(sb))
            
            if self.stream.peek() == ",":  # eat comma between operands, if there is one 
                self.stream.next_char()
                continue
        
        self.state = States.LINE_FINISHED
       
        return
    
    def _start_of_label(self):
        chars = []
        while self._is_label_char():
            chars.append(self.stream.next_char())

        label = "".join(chars)
        if not label:
            return self._print_error()

        # print("found a full label :) ", label)
        self.symbol_table[label] = self.location_counter
        self.state = States.LABEL_FOUND
        return
    
    def _label_found(self):
        self.stream.consume_white()
        ch = self.stream.peek()
        
        if ch == '.' and self.data_possible:
            self.stream.next_char()
            self.state = States.BEGIN_DIRECTIVE
            return 
        elif self._is_alpha_numeric():
            if self.data_possible: 
                self.data_possible = False
            self.state = States.BEGIN_CODE
            return 
        
        # anything else is an error 
        self._print_error()
        return
    
    def _begin_directive(self): 
        if not self.data_possible:
            self._print_error()
            return

        s = self.stream.peek(3).upper()  # looking for int, byt, bts or str
        # print(s)

        if s == Directive.INT.value:
            self.state = States.INT_DIRECTIVE
            # print("found int directive")
        elif s == Directive.BYT.value:
            self.state = States.BYT_DIRECTIVE
            # print("found byt directive")
        elif s == Directive.BTS.value:
            self.state = States.BTS_DIRECTIVE
            # print("found bts directive")
        elif s == Directive.STR.value:
            self.state = States.STR_DIRECTIVE
            # print("found str directive")
        else:
            return self._print_error()  # anything else is not a valid directive 
        
        for _ in range(3):
                self.stream.next_char()
        return;
    
    def _int_directive(self):
        self.stream.consume_white()

        if self.stream.peek() == "#":
            self.state = States.NUMBER_FOUND
            return
        elif self.stream.peek() == "\n":
            self._store_number_in_byte_list(0)
            self.state = States.LINE_FINISHED
            return

        return  

    def _number_found(self):
        digits = []
        # consume # sign
        self.stream.next_char()

        while self.stream.peek().isdigit():
            digits.append(self.stream.next_char())
        text = "".join(digits)
        try:
            n = int(text, 10) 
        except ValueError:
            return self._print_error()
        # print("spitting out n right here ")
        # print(n)
        self._store_number_in_byte_list(n)

        self.state = States.DIRECTIVE_DONE  

        return

    def _byt_directive(self):
    
        ESCAPES = {
            't': "\t",
            '\\': '\\',
            'n': '\n',
            "'": "'",
            '"': '"',
            'r': '\r',
            'b': '\b',
            }
    
        self.stream.consume_white()
        
        nxt = self.stream.peek()
        
        if nxt == "#":
            self.stream.next_char()
            nxt = self.stream.peek()
            
        # decimal case 
        if nxt.isdigit():
            digi = []
            while self._is_alpha_numeric():
                digi.append(self.stream.next_char())
            n = int("".join(digi), 10)
            self._store_number_in_byte_list(n, width=1)
            self.state = States.DIRECTIVE_DONE
            return

        # character case
        if nxt == "'":
            self.stream.next_char()  # eat opening quote 
            ch = self.stream.next_char()
            if ch == "\\":
                esc = self.stream.next_char()
                if esc not in ESCAPES:
                    return self._print_error()
                ch = ESCAPES[esc]
            
            # eat closing single quote     
            if self.stream.next_char() != "'":
                return self._print_error()
            
            self._store_number_in_byte_list(ord(ch), width=1)
            self.state = States.DIRECTIVE_DONE
            return
        
        # no operand case, default to zero 
        if nxt in ("\n", ";"):
            self._store_number_in_byte_list(0, width=1)
            self.state = States.DIRECTIVE_DONE
            return
            
        return self._print_error()
    
    def _bts_directive(self):
        # Required unsigned deicmal value. This value is the number of bytes to be allocated 
        # examples:  .BTS #25 
        #           .BTS #5
        #           a_label .BTS #20
        # Allocates the specified number of bytes in place and initializes them all to 0. If an optional label is present, the label shall be associated with the address of the first byte allocated. 
        # TODO:
        self.stream.consume_white()
        if self.stream.peek() != "#":
            return self._print_error()
        
        self.stream.next_char()
        nxt = self.stream.peek()
        
        if nxt.isdigit():
            digi = []
            while self.stream.peek().isdigit():
                digi.append(self.stream.next_char())
            width = int("".join(digi), 10)
            
            self._store_number_in_byte_list(0, width)
            self.state = States.DIRECTIVE_DONE
            return
        else:
            return self._print_error()
                
        # self.data_bytes.extend(b'\x00' * 8)

    def _str_directive(self):
        
        self.stream.consume_white()
        # print("right here, line below, we're looking at ")
        # print(self.stream.peek())
        
        if self.stream.peek() != '\"' and self.stream.peek() != "#":
            return self._print_error()
        
        str_type = self.stream.next_char()
        
        if str_type == '"': self._parse_str_string()  # string case 
        elif str_type == "#": self._parse_str_numeric()  # numeric case
        else: self._print_error()  # should never hit this, but better safe than sorry 
        
        self.state = States.DIRECTIVE_DONE
        return
    
    def _parse_str_string(self):

        ESCAPES = {
            't': "\t",
            '\\': '\\',
            'n': '\n',
            "'": "'",
            '"': '"',
            'r': '\r',
            'b': '\b'
            }
        
        # print("did we make it in here? ")
        
        # string case
        sb = []
        while True:
            ch = self.stream.peek();
            
            if ch == '"':
                break
            if ch == '\\':
                self.stream.next_char()
                esc = self.stream.next_char()
                
                if esc not in ESCAPES:
                    return self._print_error()
                sb.append(ESCAPES[esc])
            else:
                sb.append(ch)
                self.stream.next_char()
        
        if len(sb) > 255:
            self._print_error()  # broke max size rule 
        
        self.stream.next_char()  # consume closing quote 
        
        start = len(self.data_bytes)
        total = len(sb) + 2
            
        self._store_number_in_byte_list(0, total)  # allocates a number of bytes equal to the length of the string + 2
        
        self.data_bytes[start] = len(sb)  # first byte is initalized to the length of the string 
        
        for idx, ch in enumerate(sb, start=1):
            self.data_bytes[start + idx] = ord(ch)
        
        self.data_bytes[start + total - 1] = 0   
            
        # Required double quote delimited string OR a numeric literal. 255 is the max string length
        # Allocates a number of bytes equal to the length of the string + 2. The first byte is initialized to the length of the string, 
        # and the last byte to the null character. The remaining bytes in the middle are initlaized to the ascii values for the characters 
        # in the string. Escape sequences like \n must be properly handled. 
        return
    
    def _parse_str_numeric(self):
        # Required numeric literal. 255 max val of numeric literal 
        # Allocates a number of bytes equal to the numeric literal + 2. The first byte is initalized to the value of the numeric literal and the remaining bytes are initialized to zeros. 
        # TODO:
        
        sb = []
        while True:
            ch = self.stream.peek()
            if ch.isnumeric():
                sb.append(ch)
                self.stream.next_char()
            elif not ch.isnumeric():
                break
            else:
                return self._print_error()
                
        toreserve = int("".join(sb))
        
        if(toreserve > 255):
            self._print_error()  # broke max value rule 
        
        start = len(self.data_bytes)
        total = toreserve + 2
        
        self._store_number_in_byte_list(0, total)
        self.data_bytes[start] = toreserve
                    
        return

    def _directive_done(self):
        self.stream.consume_white()
        
        if self.stream.peek() != "\n" and self.stream.peek() != ";":
            print(self.stream.peek())
            self._print_error()

        self.state = States.LINE_FINISHED
        return

    def _line_finished(self):
        if self.stream.peek() == "\n":
            self.stream.next_char()
        elif self.stream.peek() == ";":
            self.stream.consume_everything_until_eol()
            self.stream.next_char()

        self.state = States.START_LINE   
        return

    # --------------------------------
    # utility helpers
    # -------------------------------- 
    def _store_number_in_byte_list(self, value: int, width: int=4) -> None:
        for b in split_to_bytes(value, width):
            self.data_bytes.append(b)
        self.location_counter += width

        # print(" emitted", list(b), "at", self.location_counter - len(b))  

        return
        
    def _print_error(self):
        self.error = True
        print(f"Assembler error encountered on line {self.stream.get_line_count()}")
        self.debugDump()
        sys.exit(2)
        return 
    
    def _is_directive_start(self) -> bool:
        if self.stream.peek() == ".":
            return True
        else: return False
    
    def _is_alpha_numeric(self) -> bool:
        ch = self.stream.peek()
        if not ch: 
            return False
        o = ord(ch)

        # hard coded alphanumeric range 
                # 0-9               a-z (uppercase)    a-z (lowercase)
        return (48 <= o <= 57) or (65 <= o <= 90) or (97 <= o <= 122)

    def _is_label_char(self) -> bool:
        ch = self.stream.peek()
        if not ch:
            return False
        o = ord(ch)

        # hard coded alphanumeric, dollar sign, or underscore 
                # 0-9               a-z (uppercase)    a-z (lowercase)    $(dollar sign) _(underscore)
        return (48 <= o <= 57) or (65 <= o <= 90) or (97 <= o <= 122) or (o == 36) or (o == 95)
   
    def debugDump(self) -> None: 

        print("Dump of current session: ")
        print("\tstream          ", self.stream)
        print("\tstate           ", self.state)
        print("\tlocation_counter", self.location_counter)
        print("\tsymbol_table    ", self.symbol_table)
        # print("\tdata_bytes hex: ", self.data_bytes.hex())
        print("\terror           ", self.error)
        print("\tdata_possible   ", self.data_possible)
        print("\tentries   ", self.entries)
        print("\tcurrent_op   ", self.current_op)
        print("\tfixups.      ", self.fixups)
        # print("\tcurrent_operands   ", self.current_operands, "\n")
        print("\n")
        print("\tdata bytes:\n")
        self.dumpBytes()
        
    def dumpBytes(self) -> None:
        width = 16
        length = len(self.data_bytes)
        
        for offset in range(0, length, width):
            chunk = self.data_bytes[offset:offset + width]
            
            hex_bytes = " ".join(f"{b:02x}" for b in chunk)
            ascii_repr = ''.join((chr(b) if 32 <= b < 127 else '.') for b in chunk)
            print(f"{offset:08X}: {hex_bytes:<{width*3}}  {ascii_repr}")

    # --------------------------------
    # ASSEMBLY STEP
    # -------------------------------- 
    def _assemble_entries(self) -> None:
        emit_handlers = {
        "JMP": self._emit_jmp,
        "JMR": self._emit_jmr,
        "BNZ": self._emit_bnz,
        "BGT": self._emit_bgt,
        "BLT": self._emit_blt,
        "BRZ": self._emit_brz,
        "MOV": self._emit_mov,
        "MOVI":self._emit_movi,
        "LDA": self._emit_lda,
        "STR": self._emit_str,
        "LDR": self._emit_ldr,
        "STB": self._emit_stb,
        "LDB": self._emit_ldb,
        "ISTR": self._emit_istr,
        "ILDR": self._emit_ildr,
        "ISTB": self._emit_istb,
        "ILDB": self._emit_ildb,
        "ADD": self._emit_add,
        "ADDI":self._emit_addi,
        "SUB": self._emit_sub,
        "SUBI":self._emit_subi,
        "MUL": self._emit_mul,
        "MULI":self._emit_muli,
        "DIV":self._emit_div,
        "SDIV":self._emit_sdiv,
        "DIVI":self._emit_divi,
        "AND":self._emit_and,
        "OR": self._emit_or,
        "CMP": self._emit_cmp,
        "CMPI": self._emit_cmpi,
        "TRP": self._emit_trp,
        "ALCI":self._emit_alci,
        "ALLC":self._emit_allc,
        "IALLC":self._emit_iallc,
        "PSHR":self._emit_pshr,
        "PSHB":self._emit_pshb,
        "POPR":self._emit_popr,
        "POPB":self._emit_popb,
        "CALL":self._emit_call,
        "RET":self._emit_ret
    }
        
        for op, operands, addr in self.entries:
            handler = emit_handlers.get(op)
           
            if not handler:
                raise ValueError(f"No emitter for operation {op}")
            handler(operands, addr)
        self._apply_fixups()
        # self.debugDump()
        return

    def _resolve_operand(self, token, patch_address):
        ESCAPES = {
            't': "\t",
            '\\': '\\',
            'n': '\n',
            "'": "'",
            '"': '"',
            'r': '\r',
            'b': '\b',
            }
        """
        valid operands are:
            -registers
            -numeric constants 
            -labels"""
            
        token = token.rstrip(",")
        
        if token.startswith("#"):  # numeric constant, token should have # in position zero, ex #1234
            return int(token[1:], 10)  # grab past the #, then return a base 10 int of that string 

        elif len(token) >= 2 and token[0] == "'" and token[-1] == "'":
            inner = token[1:-1]
            if inner.startswith("\\"):
                inner = ESCAPES.get(inner[1], None)
                if len(inner) == 1:
                    return ord(inner)
                else:
                    self._print_error()
            else:
                ch = inner
            
            if ch is None or len(ch) != 1:
                # print("bad dog ")
                return self._print_error()
            return ord(ch)
                    
        elif token.upper() in REG_MAP:  # registers are case insensitive 
            return REG_MAP[token.upper()]
        
        elif token in self.symbol_table:  # Assume that anything else is a label that we need to fix. We'll catch problems later 
            return self.symbol_table[token]
        
        self.fixups.append((patch_address, token))  # if token wasn't in the symbol table, we need to go back and fix this later. 
        
        return 0
    
    def _write_bytes(self,
                     address: int,
                     opcode: int,
                     operand_1:int,
                     operand_2:int,
                     operand_3:int,
                     immediate:int) -> None:
        
        """Lay out an 8-byte instruction at `address` 
        byteorder: 
        # [operand_value] [operand_1] [operand_2] [operand_3] [immediate_value]
         """
        self.data_bytes[address] = opcode
        self.data_bytes[address + 1] = operand_1
        self.data_bytes[address + 2] = operand_2
        self.data_bytes[address + 3] = operand_3    
        self.data_bytes[address + 4:address + 8] = split_to_bytes(immediate, 4) 
        
        return
     
    def _apply_fixups(self):
        for patch_addr, label in self.fixups:
            if label not in self.symbol_table:
                self._print_error()
            target = self.symbol_table[label]
            self.data_bytes[patch_addr:patch_addr + 4] = bytes(split_to_bytes(target, 4))
                
        # self.debugDump()
        return 
    
    def _write_bin(self, file) -> bool:
        
        with open(file, "wb") as f:
            f.write(self.data_bytes)
        f.close()
        # self.debugDump()
        return
    
    def _get_bytes(self):
        return bytes(self.data_bytes)
    
    # --------------------------------
    # Emitters
    # --------------------------------
    
    """byteorder: 
        # [operand_value] [operand_1] [operand_2] [operand_3] [immediate_value]
        
        opcode = 0  # Mnemonic.XXXX.value
        operand_1 = 0  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)    
    """

    def _emit_jmp(self, operands, addr):
        # [operand_value] [operand_1] [operand_2] [operand_3] [immediate_value]
        opcode = Mnemonic.JMP.value
        operand_1 = 0
        operand_2 = 0
        operand_3 = 0
        imm = self._resolve_operand(operands[0], addr + 4)
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)
        return

    def _emit_jmr(self, operands, addr):
        opcode = Mnemonic.JMR.value  # 
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RS  
        operand_2 = 0  # DC 
        operand_3 = 0  # DC 
        imm = 0  #       DC  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  
        return;

    def _emit_bnz(self, operands, addr):
        opcode = Mnemonic.BNZ.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RS  
        operand_2 = 0  # DC 
        operand_3 = 0  # DC 
        imm = self._resolve_operand(operands[1], addr + 4)  # ADDR    
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  

        return;

    def _emit_bgt(self, operands, addr):
        opcode = Mnemonic.BGT.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RS 
        operand_2 = 0  # DC 
        operand_3 = 0  # DC 
        imm = self._resolve_operand(operands[1], addr + 4)  #       ADDR   
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  
        return;

    def _emit_blt(self, operands, addr):
        opcode = Mnemonic.BLT.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RS 
        operand_2 = 0  # DC 
        operand_3 = 0  # DC 
        imm = self._resolve_operand(operands[1], addr + 4)  #       ADDR   
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  

        return;

    def _emit_brz(self, operands, addr):
        opcode = Mnemonic.BRZ.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RS 
        operand_2 = 0  # DC 
        operand_3 = 0  # DC  
        imm = self._resolve_operand(operands[1], addr + 4)  #       ADDR 
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  
        return;
    
    def _emit_mov(self, operands, addr):
        opcode = Mnemonic.MOV.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RD
        operand_2 = self._resolve_operand(operands[1], addr + 2)  # RS
        operand_3 = 0  # DC
        imm = 0  # DC
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)
        return

    def _emit_movi(self, operands, addr):
        opcode = Mnemonic.MOVI.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  #                                             self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #                                             self._resolve_operand(operands[X], addr + 3) 
        imm = self._resolve_operand(operands[1], addr + 4)  # IMM (NUMERIC LITERAL)     
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)        
        return

    def _emit_lda(self, operands, addr):
        opcode = Mnemonic.LDA.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = 0  # DC  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  # DC  self._resolve_operand(operands[X], addr + 3) 
        imm = self._resolve_operand(operands[1], addr + 4)  # imm address
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)        
        return

    def _emit_str(self, operands, addr):
        opcode = Mnemonic.STR.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = 0  # DC  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  # DC  self._resolve_operand(operands[X], addr + 3) 
        imm = self._resolve_operand(operands[1], addr + 4)  # ADDRESS
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_ldr(self, operands, addr):
        opcode = Mnemonic.LDR.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = 0  # dc  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  # dc  self._resolve_operand(operands[X], addr + 3) 
        imm = self._resolve_operand(operands[1], addr + 4)  # IMM ADDRESS
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_stb(self, operands, addr):
        opcode = Mnemonic.STB.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = self._resolve_operand(operands[1], addr + 4)  # IMM ADDRESS 
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_ldb(self, operands, addr):
        opcode = Mnemonic.LDB.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_istr(self, operands, addr):
        opcode = Mnemonic.ISTR.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RS  
        operand_2 = self._resolve_operand(operands[1], addr + 2)  # RG  
        operand_3 = 0  # DC 
        imm = 0  #      DC  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  

        return

    def _emit_ildr(self, operands, addr):
        opcode = Mnemonic.ILDR.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RD  
        operand_2 = self._resolve_operand(operands[1], addr + 2)  # RG  
        operand_3 = 0  # DC 
        imm = 0  #      DC  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  

        return

    def _emit_istb(self, operands, addr):
        opcode = Mnemonic.ISTB.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RS 
        operand_2 = self._resolve_operand(operands[1], addr + 2)  # RG 
        operand_3 = 0  # DC 
        imm = 0  #       DC  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  

        return

    def _emit_ildb(self, operands, addr):
        opcode = Mnemonic.ILDB.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RD 
        operand_2 = self._resolve_operand(operands[1], addr + 2)  # RG 
        operand_3 = 0  # DC 
        imm = 0  #       DC  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  

        return

    def _emit_add(self, operands, addr):
        opcode = Mnemonic.ADD.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)
        operand_3 = self._resolve_operand(operands[2], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_addi(self, operands, addr):
        opcode = Mnemonic.ADDI.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)
        operand_3 = 0  # self._resolve_operand(operands[2], addr + 3) 
        imm = self._resolve_operand(operands[2], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_sub(self, operands, addr):
        opcode = Mnemonic.SUB.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)
        operand_3 = self._resolve_operand(operands[2], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_subi(self, operands, addr):
        opcode = Mnemonic.SUBI.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)
        operand_3 = 0  # self._resolve_operand(operands[2], addr + 3) 
        imm = self._resolve_operand(operands[2], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_mul(self, operands, addr):
        opcode = Mnemonic.MUL.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)
        operand_3 = self._resolve_operand(operands[2], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_muli(self, operands, addr):
        opcode = Mnemonic.MULI.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)
        operand_3 = 0  # self._resolve_operand(operands[2], addr + 3) 
        imm = self._resolve_operand(operands[2], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_div(self, operands, addr):
        opcode = Mnemonic.DIV.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)
        operand_3 = self._resolve_operand(operands[2], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_sdiv(self, operands, addr):
        opcode = Mnemonic.SDIV.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)
        operand_3 = self._resolve_operand(operands[2], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  
        return

    def _emit_divi(self, operands, addr):
        opcode = Mnemonic.DIVI.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)
        operand_3 = 0  # self._resolve_operand(operands[2], addr + 3) 
        imm = self._resolve_operand(operands[2], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)   
        return

    def _emit_and(self, operands, addr):
        # ADD RD RS1 RS2 DC
            
        opcode = Mnemonic.AND.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = self._resolve_operand(operands[2], addr + 3)  #  self._resolve_operand(operands[X], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)    
        return

    def _emit_or (self, operands, addr):
        # OR RD RS1 RS2 DC 
        opcode = Mnemonic.OR.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = self._resolve_operand(operands[2], addr + 3)  #  self._resolve_operand(operands[X], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)    
        return
    
    def _emit_cmp(self, operands, addr):
        opcode = Mnemonic.CMP.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RD 
        operand_2 = self._resolve_operand(operands[1], addr + 2)  # RS1 
        operand_3 = self._resolve_operand(operands[2], addr + 3)  # RS2  
        imm = 0  #       DC  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  

        return

    def _emit_cmpi(self, operands, addr):
        opcode = Mnemonic.CMPI.value  # Mnemonic.XXXX.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  # RD  
        operand_2 = self._resolve_operand(operands[1], addr + 2)  # RS1 
        operand_3 = 0  # DC  
        imm = self._resolve_operand(operands[2], addr + 4)  #      IMM  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  

        return

    def _emit_trp(self, operands, addr):
        opcode = Mnemonic.TRP.value
        operand_1 = 0  # self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  # self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  # self._resolve_operand(operands[X], addr + 3) 
        imm = self._resolve_operand(operands[0], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm) 
        return

    def _emit_alci (self, operands, addr):
    
        # [operand_value] [operand_1] [operand_2] [operand_3] [immediate_value]
        
        opcode = Mnemonic.ALCI.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)    
        
        return

    def _emit_allc (self, operands, addr):

        opcode = Mnemonic.ALLC.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)    
        return

    def _emit_iallc (self, operands, addr):

        # [operand_value] [operand_1] [operand_2] [operand_3] [immediate_value]
        
        opcode = Mnemonic.IALLC.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)
        operand_2 = self._resolve_operand(operands[1], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm) 
        return

    def _emit_pshr (self, operands, addr):

        opcode = Mnemonic.PSHR.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)    
        return

    def _emit_pshb (self, operands, addr):

        opcode = Mnemonic.PSHB.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)    
        return

    def _emit_popr (self, operands, addr):

        opcode = Mnemonic.POPR.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)    
        return

    def _emit_popb (self, operands, addr):

        opcode = Mnemonic.POPB.value
        operand_1 = self._resolve_operand(operands[0], addr + 1)  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)    
        return

    def _emit_call (self, operands, addr):

        # [operand_value] [operand_1] [operand_2] [operand_3] [immediate_value]
        
        opcode = Mnemonic.CALL.value
        operand_1 = 0  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = self._resolve_operand(operands[0], addr + 4)  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm)  
        return

    def _emit_ret (self, operands, addr):
   
        # [operand_value] [operand_1] [operand_2] [operand_3] [immediate_value]
        
        opcode = Mnemonic.RET.value
        operand_1 = 0  #  self._resolve_operand(operands[X], addr + 1)
        operand_2 = 0  #  self._resolve_operand(operands[X], addr + 2)
        operand_3 = 0  #  self._resolve_operand(operands[X], addr + 3) 
        imm = 0  #        self._resolve_operand(operands[1], addr + 4)  
        self._write_bytes(addr, opcode, operand_1, operand_2, operand_3, imm) 
        return


# --------------------------------
# cli helpers
# --------------------------------
def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="assemble a single .asm source file")
    parser.add_argument("input",
                        metavar="file.asm",
                        help="assembly source file (must end in .asm)")
    return parser.parse_args(argv)

# def configure_logging(verbosity):
#    level = logging.warning - 10 * min(verbosity, 2)  # warn, info, debug
#    logging.basic_config(level=level, format="%(levelname)s | %(message)s")

# ------------------------------------------------------------------------------------------------
# other utility helpers that didn't make sense to put in a class 
# ------------------------------------------------------------------------------------------------


def validate_file_name(name):
    # forces the entire string to match *.asm, case-insensitive
    return bool(re.fullmatch(r'^.+\.asm$', name, re.IGNORECASE))


def split_to_bytes(value: int, width: int) -> list[int]:
    """given an int, split it into an ints worth of bytes"""
    return [(value >> (8 * i)) & 0xFF for i in range(width)]

# --------------------------------
# entry point
# --------------------------------


def main(argv=None):
    args = parse_args(argv)
    infilename = args.input
    
    if not validate_file_name(infilename):
        print("usage: python3 asm4380.py input_file.asm")
        sys.exit(1)  # requirement 6

    outfilename = infilename.removesuffix(".asm")
    outfilename += ".bin"

    p = Assembler(infilename)
    p.run()
    p._write_bin(outfilename)
    # print(outfilename)
    
    return 0


if __name__ == "__main__":
    main()
