from gen_format_conv import *

for count in [1,2,3,4]:
    print("if (")
    for n in [8, 16, 32, 64]:
        lowest = Info(n, "Uint")
        highest = Info(n, "Sfloat") if n !=8 else Info(n, "Sint")
        print('(', mk_fmt(count, lowest), '<', 'fmt && fmt <', mk_fmt(count, highest), ') ||')
    print(f'false)\n  return {count};')

