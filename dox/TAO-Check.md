# Threadsafe Methods in TAO Check

The Thread Affine Objects (TAO) check in `mulle_objc_object_taocheck_call` considers the following methods as threadsafe:

1. **Memory Management Methods**
   - `release`
   - `retain`
   - `autorelease`

2. **Methods with Zero Retain Count**
   - Any method called on an object with a retain count of 0

3. **Methods on Objects with Matching Thread**
   - Any method called on an object where the object's thread matches the current thread

4. **Non-Thread Affine Methods**
   - Methods that are not marked as thread affine (i.e., `_mulle_objc_method_is_threadaffine` returns false)

5. **Initialization and Deallocation Methods**
   - Methods in the `init` family
   - Methods in the `dealloc` family

6. **Getter Methods for Read-Only Properties**
   - Getter methods for properties marked as read-only

7. **Forward Methods**
   - Methods with the `MULLE_OBJC_FORWARD_METHODID`

## Special Considerations

- The thread safety of a method can vary between classes. The check looks up the method as declared by the specific class of the object to determine its thread affinity.
- Properties are checked for read-only status only if the class is an infraclass and a corresponding property ID is found for the method ID.
- The TAO check is skipped if the universe is in the process of deinitializing.

Note: This documentation reflects the behavior as implemented in the provided code snippet. Always refer to the latest documentation and code for the most up-to-date information.
