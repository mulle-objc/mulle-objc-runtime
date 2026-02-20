// Test for signature parsing utilities
#include "include.h"

int main(void)
{
    const char *signature = "@@:";
    struct mulle_objc_signatureenumerator enumerator;
    struct mulle_objc_typeinfo info;
    
    mulle_printf("Test: Signature parsing\n");
    mulle_printf("Signature: %s\n", signature);
    
    // Parse the signature using the enumerator
    enumerator = mulle_objc_signature_enumerate((char *)signature);
    
    // Get return type (first element)
    _mulle_objc_signatureenumerator_rval(&enumerator, &info);
    mulle_printf("Return type: %s\n", info.type);
    
    // Count parameters (skip self and _cmd which are implicit)
    int param_count = 0;
    while (_mulle_objc_signatureenumerator_next(&enumerator, &info))
    {
        param_count++;
        mulle_printf("Parameter %d type: %s\n", param_count - 1, info.type);
    }
    
    mulle_objc_signatureenumerator_done(&enumerator);
    
    return 0;
}