import unittest
import io
import os
import sys
import tempfile

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname("assembler/asm4380.py"), "..")))

from assembler.asm4380 import Assembler
from assembler.asm4380 import REG_MAP


class StringDriver:

    def __init__(self, text:str):
        self.tmp = tempfile.NamedTemporaryFile(mode="w+", suffix=".asm", delete=False)
        self.tmp.write(text)
        self.tmp.flush()
        self.tmp.close()
        
        self.parser = Assembler(self.tmp.name)
        
    def assemble_to_bytes(self) -> bytes:
        self.parser.run()
        return self.parser._get_bytes()
    
    def cleanup(self):
        try:
            os.remove(self.tmp.name)
        except OSError:
            pass

    
class TestByteDirective(unittest.TestCase):
    
    def setUp(self):
        self.drivers = []
        
    def tearDown(self):
        for d in self.drivers:
            d.cleanup()
            
    def new_driver(self, text):
        d = StringDriver(text)
        self.drivers.append(d)
        return d

    def test_decimal_with_hash(self):
        drv = self.new_driver(".BYT #42\n")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x2A])
        
    def test_char_literal(self):
        drv = self.new_driver(r".BYT '\n'")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x0A])


class TestEmitters(unittest.TestCase):

    def setUp(self):
        self.drivers = []
    
    def tearDown(self):
        for d in self.drivers:
            d.cleanup()
            
    def new_driver(self, text):
        d = StringDriver(text)
        self.drivers.append(d)
        return d 
    
    def test_AND_with_registers(self):
        drv = self.new_driver("\tAND R0 R1 R2")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x1B, 0x00, 0x01, 0x02, 0, 0, 0, 0])
    
    def test_AND_with_numerics(self):
        drv = self.new_driver("\tAND #0 #1 R2")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x1B, 0x00, 0x01, 0x02, 0, 0, 0, 0])
    
    def test_AND_with_label(self):
        drv = self.new_driver("ANDLABEL AND #0 #1 R2")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x1B, 0x00, 0x01, 0x02, 0, 0, 0, 0])
        
    def test_OR_with_registers(self):
        drv = self.new_driver("\tOR R0 R1 R2")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x1C, 0x00, 0x01, 0x02, 0, 0, 0, 0])
    
    def test_OR_with_numerics(self):
        drv = self.new_driver("\tOR #0 #1 R2")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x1C, 0x00, 0x01, 0x02, 0, 0, 0, 0])
    
    def test_OR_with_label(self):
        drv = self.new_driver("ORLABEL OR #0 #1 R2")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x1C, 0x00, 0x01, 0x02, 0, 0, 0, 0])
        
    def test_pshr(self):
        drv = self.new_driver("\tpshr r0")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x23, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0])  
    
    def test_pshb(self):
        drv = self.new_driver("\tpshb r0")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x24, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0])    
    
    def test_popr(self):
        drv = self.new_driver("\tpopr r0")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x25, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0])    
    
    def test_popb(self):
        
        drv = self.new_driver("\tpopb r0")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x26, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0])    
    
    def test_call(self):
        teststring = "start call func\n \ttrp #0\nfunc trp #0"
        
        drv = self.new_driver(teststring)
        out = drv.assemble_to_bytes()
        
        call_instr = out[4:4 + 8]
        opcode, r1, r2, r3, imm0, imm1, imm2, imm3 = call_instr     
        self.assertEqual(opcode, 0x27)
        self.assertEqual(r1, 0x00)    
        addr = imm0 | (imm1 << 8) | (imm2 << 16) | (imm3 << 24)
        
        self.assertEqual(addr, 20)

    def test_ret(self):
        drv = self.new_driver("\tret ")
        out = drv.assemble_to_bytes()
        self.assertEqual(list(out[4:]), [0x28, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0])    
        
    def test_alci(self):
        drv = self.new_driver("\talci r0, #255")
        out = drv.assemble_to_bytes() 
        instr = out[4:4 + 8]
        opcode, r1, r2, r3, imm0, imm1, imm2, imm3 = instr 
        self.assertEqual(opcode, 0x20)
        self.assertEqual(r1, 0x00)
        imm = imm0 | (imm1 << 8) | (imm2 << 16) | (imm3 << 24)
        self.assertEqual(imm, 255)
        
    def test_allc(self):
        teststring = "n .INT #64\n\tallc r2, n"
        
        drv = self.new_driver(teststring)
        out = drv.assemble_to_bytes()
        
        instr = out[8:16]
        opcode, rd, r2, r3, imm0, imm1, imm2, imm3 = instr     
        
        self.assertEqual(opcode, 0x21)
        self.assertEqual(rd, REG_MAP["R2"])    
        imm = imm0 | (imm1 << 8) | (imm2 << 16) | (imm3 << 24)
        
        self.assertEqual(imm, 4)
        
    def test_bts_without_label_allocate_3_bytes(self):
        src = "\t.BTS #3\t\n"
        drv = StringDriver(src)
        out = drv.assemble_to_bytes()
        
        self.assertEqual(len(out), 4 + 3)
        self.assertEqual(out[4:], bytearray([0, 0, 0]))
        
    def test_bts_with_label_allocate_3_bytes(self):
        src = "LABELHERE .BTS #3\t\n"
        drv = StringDriver(src)
        out = drv.assemble_to_bytes()
        
        self.assertEqual(len(out), 4 + 3)
        self.assertEqual(out[4:], bytearray([0, 0, 0]))
        
    def test_bts_with_label_correct_address(self):
        src = "labelhere .BTS #13"
        drv = StringDriver(src)
        out = drv.assemble_to_bytes()
        
        self.assertEqual(len(out), 4 + 13)
        self.assertEqual(out[4:], bytearray(13))
        
        symboltable = drv.parser.symbol_table
        self.assertEqual(symboltable.get("labelhere"), 4)
        
    def test_str_string_without_labels(self):
        src = '\t.STR "beavis and butthead"'
        drv = StringDriver(src)
        out = drv.assemble_to_bytes()
        
        self.assertEqual(len(out), 4 + 19 + 2)  # beavis and butthead is 19 characters, plus 2 additional bytes from the str overhead
        
        s = "beavis and butthead"
        
        expected = ([len(s)] + [ord(c) for c in s] + [0])
        self.assertEqual(list(out[4:]), expected)

    def test_str_string_with_label(self):
        src = 'beavis\t.STR\t"beavis and butthead"'
        drv = StringDriver(src)
        out = drv.assemble_to_bytes()
        
        self.assertEqual(len(out), 4 + 19 + 2)  # beavis and butthead is 19 characters, plus 2 additional bytes from the str overhead
        
        s = "beavis and butthead"
        
        expected = ([len(s)] + [ord(c) for c in s] + [0])
        self.assertEqual(list(out[4:]), expected)
        st = drv.parser.symbol_table
        self.assertEqual(st.get("beavis"), 4)
        
    def test_str_with_numeric_no_label(self):
        src = "\t.STR\t#4\n"
        drv = StringDriver(src)
        out = drv.assemble_to_bytes()
        
        self.assertEqual(len(out), 4 + 4 + 2)  # fed numeric of 4, plus 4 padding for location of first address, plus the additional 2 bytes needed 
        expected = (bytearray(10))
        expected[4] = 0x04
        
        self.assertEqual(out, expected) 
        
    def test_str_too_large_number(self):
        src = "\t.STR\t#256\n"  # oob
        drv = StringDriver(src)
        
        with self.assertRaises(SystemExit) as cm:
            out = drv.assemble_to_bytes()
        self.assertEqual(cm.exception.code, 2)
            
    def test_str_numeric_with_label(self):
        src = "beavis\t.STR\t#4"
        drv = StringDriver(src)
        out = drv.assemble_to_bytes()
        
        self.assertEqual(len(out), 4 + 4 + 2)  # fed numeric of 4, plus 4 padding for location of first address, plus the additional 2 bytes needed 
        expected = (bytearray(10))
        expected[4] = 0x04
        
        st = drv.parser.symbol_table
        self.assertEqual(st.get("beavis"), 4)

    
if __name__ == "__main__":
    unittest.main()
