import math

MAX_FFT_SIZE = 2048
MIN_FFT_SIZE = 16

HALF_SIZE = int(MAX_FFT_SIZE / 2)
MAX_BIT_SIZE = int(math.log2(HALF_SIZE))

def inverse(val, bitsize):
    bitpattern = ("{0:0" + str(bitsize) + "b}").format(val)
    rev = bitpattern[::-1]
    return int(rev, 2)
    
res = []
for i in range(1, HALF_SIZE + 1):
    res.append(inverse(i, MAX_BIT_SIZE))

out = ""

out += "\n   function Bitrev_Index_First (FFT_Size : Natural) return Natural\n"
out += "   is (case FFT_Size is\n"
size = MAX_FFT_SIZE
first = 1
while size >= MIN_FFT_SIZE:
    out += "          when %d => %d,\n" % (size, first - 1)
    first *= 2
    size /= 2
out += "          when others => raise Program_Error with \"Invalid FFT Size\");\n"

out += "\n   function Bitrev_Index_Step (FFT_Size : Natural) return Natural\n"
out += "   is (case FFT_Size is\n"
size = MAX_FFT_SIZE
first = 1
while size > 1:
    out += "          when %d => %d,\n" % (size, first)
    first *= 2
    size /= 2
out += "          when others => raise Program_Error with \"Invalid FFT Size\");\n"

out += "\n   Bitrev_Table : constant array (Natural range 0 .. %d) of Natural :=\n" % (HALF_SIZE - 1)
out += "     (\n"
out += "      "
for i in range(len(res)):
    out += "16#%03X#," % res[i]
    if i % 10 == 9:
        out += '\n      '
    else:
        out += ' '
out = out.rstrip(", ")
out += ");\n"

size = MAX_FFT_SIZE
while size >= MIN_FFT_SIZE:
    size = int(size)
    out += f"\n   Window_Hanning_{size} : constant Mono_Buffer :=\n"
    out += "     ("
    for i in range(size):
        val = 0.5 * (1.0 - math.cos(2.0 * math.pi * (i / size))) * 32767
        out += f"{int(val)},"
        if i % 10 == 9:
            out += '\n      '
        else:
            out += ' '
    out = out.rstrip(", ")
    out += ");\n"
    size /= 2

out += "\n"
out += "   ------------------\n"
out += "   -- Apply_Window --\n"
out += "   ------------------\n"
out += "\n"
out += "   procedure Apply_Window (Input : in out Mono_Buffer) is\n"
out += "   begin\n"
out += "      case Input'Length is\n"
size = MAX_FFT_SIZE
while size >= MIN_FFT_SIZE:
    size = int(size)
    out += f"         when {size} => Apply_Window (Input, Window_Hanning_{size});\n"
    size /= 2
out += "         when others => raise Program_Error with \"Invalid FFT size\";\n"
out += "      end case;\n"
out += "   end Apply_Window;\n"


print(out)
