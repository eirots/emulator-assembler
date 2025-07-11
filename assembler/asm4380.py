import logging
import argparse
import re
import sys
# TODO: YOU WERE LAST WORKING IN THE _BEGIN_CODE FUNCTION, DONT FORGET DAT BAYBEEE 

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
    NUMBER_FOUND = auto()
    DIRECTIVE_FOUND = auto()
    DIRECTIVE_DONE = auto()
    LINE_FINISHED = auto() 
    BEGIN_CODE = auto()
    GATHER_TOKENS = auto()


class Directive(Enum): 
    INT = "INT"
    BYT = "BYT"


class Mnemonic(IntEnum):
    JMP = 0x01
    MOV = 0x07
    MOVI = 0x08
    LDA = 0x09
    STR = 0x0A
    LDR = 0x0B
    STB = 0x0C
    LDB = 0x0D
    ADD = 0x12
    ADDI = 0x13
    SUB = 0x14
    SUBI = 0x15
    MUL = 0x16
    MULI = 0x17
    DIV = 0x18
    SDIV = 0x19
    DIVI = 0x1A
    TRP = 0x1F


VALID_OPS = {
        "JMP",
        "MOV",
        "MOVI",
        "LDA",
        "STR",
        "LDR",
        "STB",
        "LDB",
        "ADD",
        "ADDI",
        "SUB",
        "SUBI",
        "MUL",
        "MULI",
        "DIV",
        "SDIV",
        "DIVI",
        "TRP"
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
        while ch in (" ", "\t"):
            self.next_char()
            ch = self.peek()
        return
    
    def consume_everything_until_eol(self):
        ch = self.peek()
        while not ch == "\n":
            self.next_char()
            ch = self.peek()
        return


class Parser:

    def __init__(self, path):
        self.stream = CharStream(path)
        self.state = States.START_LINE
        self.location_counter = 0  # pointer to end of bytes 
        self.symbol_table = {}
        
        self.data_bytes = bytearray()
        self.fixups = []
        
        self.error = False
        self.data_possible = True  # turned off when first encountering a code portion
        
    def run(self) -> int:
        handlers = {
            States.START_LINE: self._start_line,
            States.START_OF_LABEL: self._start_of_label,
            States.LABEL_FOUND: self._label_found,
            States.BEGIN_DIRECTIVE: self._begin_directive,
            States.INT_DIRECTIVE: self._int_directive,
            States.BYT_DIRECTIVE: self._byt_directive,
            States.NUMBER_FOUND: self._number_found,
            States.LINE_FINISHED: self._line_finished,
            States.DIRECTIVE_DONE: self._directive_done,
            States.BEGIN_CODE: self._begin_code
            }
        
        while not self.stream.eof() and not self.error: 
            handlers[self.state]()
        
        if self.error:
            return self.debugDump()
        
    # --------------------------------
    # constants
    # --------------------------------

    # --------------------------------
    # core logic
    # --------------------------------
    def _start_line(self): 
        # we are concerned with two types of instructions here. 
        # DIRECTIVE -> [optional_label] <.directive> [optional_value] [optional_comment]
        # CODE      -> [optional_lable] <operator> <operand_list> [optional_comment]
        
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
            if self.data_possible: self.data_possible = False
            self.state = States.BEGIN_CODE
            # self._print_error()                 
            return        
        
        # anything else is an invalid instruction
        
        self._print_error()          
    
    def _begin_code(self) -> None: 
        s = ""
        
        if self.stream.peek(4)[3] == " ":
            s = "".join(self.stream.next_n(3))
        else:
            s = "".join(self.stream.next_n(4))
        
        self.data_bytes.extend(b'\x00' * 8)
        self.location_counter += 8  # reserving space for the future 
        
        if s not in VALID_OPS:
            self._print_error()
        
        self.stream.consume_white()
        
        return

    def _gather_tokens(self):
        
        # TODO LAST WORKING HERE, IF YOU TRY TO CHECK WHERE YOU WERE LAST WORKING, IT WAS EXACTLY HERE 
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
            if self.data_possible: self.data_possible = False
            self.State = States.BEGIN_CODE
            return 
        
        # anything else is an error 
        self._print_error()
        return
    
    def _begin_directive(self): 
        if not self.data_possible:
            self._print_error()
            return

        s = self.stream.peek(3).upper()  # looking for int or byt
        # print(s)

        if s == Directive.INT.value:
            for _ in range(3):
                self.stream.next_char()
            self.state = States.INT_DIRECTIVE
            # print("found int directive")
            return
        elif s == Directive.BYT.value:
            for _ in range(3):
                self.stream.next_char()
            self.state = States.BYT_DIRECTIVE
            # print("found byt directive")
            return 
        else:
            return self._print_error()
    
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
        # decimal case 
        if nxt.isdigit():
            digi = []
            while self._is_alpha_numeric():
                digi.append(self.stream.next_char())
            n = int("".join(digi), 10)
            self._store_byt_in_byte_list(n, width=1)
            self.state = States.LINE_FINISHED
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
        o = ord(ch)

        # hard coded alphanumeric range 
                # 0-9               a-z (uppercase)    a-z (lowercase)
        return (48 <= o <= 57) or (65 <= o <= 90) or (97 <= o <= 122)

    def _is_label_char(self) -> bool:
        ch = self.stream.peek()
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
        self.dumpBytes()
        print("\tdata_bytes hex: ", self.data_bytes.hex())
        print("\terror           ", self.error)
        print("\tdata_possible   ", self.data_possible, "\n")
        
    def dumpBytes(self) -> None:
        print("\t\tindex\t byte")
        for i, b in enumerate(self.data_bytes):
            print(f"\t\t{i:3}\t 0x{b:02X}")

    
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
    filename = args.input
    
    if not validate_file_name(filename):
        print("usage: python3 asm4380.py input_file.asm")
        sys.exit(1)  # requirement 6

    # TODO: finish this section. make sure when exiting the data section that you toggle the data possible boolean to off. 
    # error boolean checking before switch, also have an int for error cause 
    p = Parser(filename)
    p.run()
    
    logging.info("done.")
    print("i am alive")


if __name__ == "__main__":
    main()
