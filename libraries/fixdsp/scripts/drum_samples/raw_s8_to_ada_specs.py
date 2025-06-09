#!/usr/bin/python3

import sys
import os
from pathlib import Path

input_file = sys.argv[1]
sample_rate = sys.argv[2]
src_stem = Path(input_file).stem
output_file = os.path.join("..", "..", "src", f"samples_SR{sample_rate}",
                           f"tresses-samples-sample_{src_stem}.ads")

out =  f"package Tresses.Samples.Sample_{src_stem}\n"
out += "with Preelaborate\n"
out += "is\n"
out += "   pragma Style_Checks (Off);\n"
out += "   Sample : aliased constant S8_Array := (\n"


with open(input_file, 'rb') as f:
   while 1:
      byte_s = f.read(1)
      if not byte_s:
         break
      byte = byte_s[0]
      if byte >= 128:
          out += str(byte - 256) + ",\n"
      else:
          out += str(byte) + ",\n"
          
out += "0);\n"
out += f"end Tresses.Samples.Sample_{src_stem};\n"

print(output_file)
with open(output_file, 'w') as file:
    file.write(out)
