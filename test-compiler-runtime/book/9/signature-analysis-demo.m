// Test for advanced signature analysis
#include "include.h"

int main(void)
{
    const char *signature = "v@:@i";
    struct mulle_objc_signatureenumerator enumerator;
    struct mulle_objc_typeinfo info;
    unsigned int param_count = 0;
    
    mulle_printf("Test: Signature analysis\n");
    mulle_printf("Signature: %s\n", signature);
    
    // Skip return type to get to parameters
    enumerator = mulle_objc_signature_enumerate((char *)signature);
    
    // Enumerate parameters (self, _cmd, then actual parameters)
    while (_mulle_objc_signatureenumerator_next(&enumerator, &info))
    {
        if (param_count >= 2) // Skip self and _cmd
        {
            mulle_printf("Parameter %d type: %s\n", param_count - 2, info.type);
        }
        param_count++;
    }
    
    // Get return type
    _mulle_objc_signatureenumerator_rval(&enumerator, &info);
    mulle_printf("Return type: %s\n", info.type);
    
    mulle_objc_signatureenumerator_done(&enumerator);
    
    return 0;
}