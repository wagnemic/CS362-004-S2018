import sys

from ansi2html import Ansi2HTMLConverter

conv = Ansi2HTMLConverter()

with open('tarantula.out', 'r') as myfile:
    tarantulaANSI = myfile.read()
    
print(conv.convert(tarantulaANSI))
