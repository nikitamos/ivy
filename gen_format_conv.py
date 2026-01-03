from dataclasses import dataclass

@dataclass
class Info:
    width: int
    postfix: str


postfixmap = {
    "SByte": Info(8, "Uint"),
    "UByte": Info(8, "Uint"),
    "Short": Info(16, "Sint"),
    "UShort": Info(16, "Uint"),
    "Int": Info(32, "Sint"),
    "UInt": Info(32, "Uint"),
    "Int64": Info(64, "Sint"),
    "UInt64": Info(64, "Uint"),
    "Float": Info(32, "Sfloat"),
    "Double": Info(64, "Sfloat")
}

def mk_i(count, width):
    width = str(width)
    res = ""
    for j in 'RGBA'[:count]:
        res += j + width
    return res

def mk_fmt(count: int, ty: Info):
    return f'vk::Format::e{mk_i(count, ty.width)}{ty.postfix}'

nextsw = lambda ty: "switch (override_vecsize) {" + ';\n'.join([f"case {i}: return {mk_fmt(i, ty)}" for i in [1,2,3,4] ]) + ';\n}'

if __name__ == '__main__':
    print("switch (type.basetype) {")
    for ty in postfixmap:
        print(f"case spc::SPIRType::{ty}:")
        print(nextsw(postfixmap[ty]))
    print(
"""
default: throw std::runtime_error("don't know format for base type " + std::to_string(type.basetype));
"""
    )
    print("}")