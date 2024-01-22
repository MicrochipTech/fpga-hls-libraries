def fix2float(fixed_point_num, fractional_bits):
    f : float = fixed_point_num / (2.0 ** fractional_bits)
    return f

def complement(val, width):
    return 0 if val == 0 else (1 << width) - val;

def getArraySize(t):
    return int(t.sizeof / t.target().sizeof)


class HLS_ap_ufixed_printer:
    def __init__(self, val, typedef):
        self.val = val
        self.typedef = typedef

    def to_string (self):
        if self.val.type.code == gdb.TYPE_CODE_PTR:
            if self.typedef:
                valType = str(self.val.type.target().strip_typedefs())
            else:
                valType = str(self.val.type.target())
        elif self.typedef:
            valType = str(self.val.type.strip_typedefs())
        else:
            valType = str(self.val.type)


        W : int = int(valType.split("<")[1].split(",")[0].replace("u",""))
        IW : int = int(valType.split("<")[1].split(",")[1].replace(" ",""))

        # If to_double() is not used in the code it gets optimized away. Therefore,
        # calling to_double() from gdb will not work.
        # valAddr = str(hex(self.val.address))
        # eval_string = "(*("+valType+"*)("+valAddr+")).to_double()"
        # floatVal = gdb.parse_and_eval(eval_string)
        # return gdb.parse_and_eval(eval_string)
        
        # TODO. Need to parse the Number of words & concatenate them. For now only
        # accessing one word: [0]
        rawBits = self.val['data_']['words_'][0]
        
        hexRawBits = str((hex(rawBits)))
        floatVal = fix2float(rawBits,W-IW)

        # Create an output file  for debugging the pretty-printer. There's no
        # printf() or step-by-step dbg for this.
        # with open("dbgPrettyPrinter." + self.__class__.__name__ + ".txt", "w") as output_file:
        #     try:
        #         outStr : str = ""
        #         outStr += ", " + hexRawBits
        #         # outStr += ", " + eval_string
        #         outStr += ", " + str(floatVal)
        #         outStr += ", " + str(valType)
        #         outStr += ", W:" + str(W) + ", IW:" + str(IW)

        #         output_file.write(outStr)
        #     except Exception as e:
        #         print(str(e))

        retStr = str(floatVal) + " [" + hexRawBits + "] <W:" + str(W) + ",IW:" + str(IW) + ">"
        return retStr

class HLS_ap_fixed_printer:
    def __init__(self, val, typedef = False):
        self.val = val
        self.typedef = typedef

    def to_string (self):
        if self.val.type.code == gdb.TYPE_CODE_PTR:
            if self.typedef:
                valType = str(self.val.type.target().strip_typedefs())
            else:
                valType = str(self.val.type.target())
        elif self.typedef:
            valType = str(self.val.type.strip_typedefs())
        else:
            valType = str(self.val.type)

        W : int = int(valType.split("<")[1].split(",")[0].replace("u",""))
        IW : int = int(valType.split("<")[1].split(",")[1].replace(" ",""))
        FW : int = W-IW 

        nElem = getArraySize(self.val.type)

        if nElem == 1:
            rawBits = self.val['data_']['words_'][0]
        else:
            # TODO. Need to create a loop here and iterate over the array
            # elements. Perhaps refactor the code to avoid code duplication, 
            # For example:  idx=0..nElem-1 
            #   rawBits[idx]['data_']['words_'][0]
            #   retStr = parseBits(rawBits)
            # For now we only access the first element of the array.
            rawBits = self.val[0]['data_']['words_'][0]

        sign = rawBits >> (W-1);
        hexRawBits = str((hex(rawBits)))

        intPart=0
        fracPart=0
        oneMore=0
        val = rawBits

        # Negative numbers are encoded as 2's complement and need to be converted
        # back to sign + positive magnitude
        if sign:
            intPart  = rawBits >> FW
            fracPart = rawBits & ((1<<FW)-1)
            oneMore = 1 if (fracPart > 0) else 0
            intPart = complement(intPart + oneMore, IW);
            fracPart = complement(fracPart, FW);
            val = (intPart << FW) | fracPart

        floatVal = fix2float(val,W-IW)

        # Create an output file  for debugging the pretty-printer. There's no
        # printf() or step-by-step dbg for this. 
        # with open("dbgPrettyPrinter." + self.__class__.__name__ + ".txt", "w") as output_file:
        #     try:
        #         outStr : str = ""
        #         outStr += ", h:" + hexRawBits
        #         outStr += ", s:" + str(sign)
        #         outStr += ", ip:" + str(intPart)
        #         outStr += ", fp:" + str(fracPart)
        #         outStr += ", +1:" + str(oneMore)
        #         outStr += ", val:" + str(val)
        #         # outStr += ", " + eval_string
        #         outStr += ", f:" + str(floatVal)
        #         if nElem > 1:
        #             outStr += ", V:["+ str(nElem) + "]"
        #         else:
        #             outStr += ", V:[Scalar]"
        #         outStr += ", T:" + str(valType)
        #         outStr += ", W:" + str(W) + ", IW:" + str(IW)

        #         output_file.write(outStr)
        #     except Exception as e:
        #         print(str(e))

        retStr = str(floatVal) + " [" + hexRawBits + "] <W:" + str(W) + ",IW:" + str(IW) + ">"
        retStr = "-" + retStr if sign else retStr 
        return retStr
    
def hls_lookup_type (val):

    if val.type.code == gdb.TYPE_CODE_PTR:
        if "hls::ap_ufixpt" in str(val.type.target()):
            return HLS_ap_ufixed_printer(val, typedef=False)
        
        if "hls::ap_ufixpt" in str(val.type.target().strip_typedefs()):
            return HLS_ap_ufixed_printer(val, typedef=True)

        if "hls::ap_fixpt" in str(val.type.target()):
            return HLS_ap_fixed_printer(val, typedef=False)
        
        if "hls::ap_fixpt" in str(val.type.target().strip_typedefs()):
            return HLS_ap_fixed_printer(val, typedef=True)

    if "hls::ap_ufixpt" in str(val.type):
        return HLS_ap_ufixed_printer(val, typedef=False)

    if "hls::ap_ufixpt" in str(val.type.strip_typedefs()):
        return HLS_ap_ufixed_printer(val, typedef=True)

    if "hls::ap_fixpt" in str(val.type):
        return HLS_ap_fixed_printer(val, typedef=False)

    if "hls::ap_fixpt" in str(val.type.strip_typedefs()):
        return HLS_ap_fixed_printer(val, typedef=True)
    
gdb.pretty_printers.append(hls_lookup_type)
