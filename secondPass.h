#ifndef SECONDPASS_H
#define SECONDPASS_H

#include "utils.h"
#include "firstPass.h"
memory replaceSymbolWithAddr(symbolNode** headIC,symbolNode** headDC);
memory secondPass(symbolNode** headIC,symbolNode** headDC);
#endif 