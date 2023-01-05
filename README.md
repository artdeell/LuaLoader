# LuaLoader
This mod for Canvas is intended to make it easier for Canvas mod developers to test and prototype their ideas using the Lua scripting language.  
The mod gives access to most of the Canvas APIs.
# Documentation
## cipher table
The `cipher` table is the table that contains all of the static APIs of Canvas.
### cipher.getGameVersion()
`number cipher.getGameVersion`  
returns the game's version number  
### cipher.isGameBeta()
`boolean cipher.isGameBeta`  
returns if the game that is currently running is a beta version of the game  
### cipher.get_libBase()
`light_userdata cipher.get_libBase`  
returns the pointer to the start of the library in memory as light userdata  
### cipher.get_libName()
`string cipher.get_libName`  
returns the name of the game's library (libBootloader.so)  
### cipher.CipherScan()
`light_userdata cipher.CipherScan(string pattern, string mask)`  
pattern - the pattern, consisting of raw characters  
mask - the mask for the pattern  
The pattern's characters will be masked with characters from the mask (`x` in mask meaning an exact match, `?` meaning an "any char" match), and if a result matching this would be found, the function will return the start of that result as light userdata, or nil if it was not found
returns result of the pattern scan as light userdata or nil if not found  
## lu table
the `lu` table contains functions to operate with light userdata
### lu.fromhex()
`light_userdata lu.fromhex(string hex)`  
hex - the string of hexadecimal characters  
returns string `hex` converted to light userdata  
### lu.add()
`light_userdata lu.add(light_userdata data, integer x)`  
data - source light userdata  
x - the integer to add to light userdata  
returns light userdata, equal to `data`+`x`   
### lu.multiply()
`light_userdata lu.multiply(light_userdata data, integer x)`  
data - source light userdata  
x - the integer to multiply light userdata by  
returns light userdata, equal to `data`*`x`   
### lu.divide()
`light_userdata lu.divide(light_userdata data, integer x)`  
data - source light userdata  
x - the integer to divide light userdata by  
returns light userdata, equal to `data`/`x`   
### lu.readint<8/16/32/64>()
`integer lu.readint<8/16/32/64>(light_userdata where_from)`  
where_from - the light userdata from which the int<8/16/32/64> will be read  
returns an integer read from memory that `where_from` points to  
### lu.read<float/double>()
`number lu.read<float/double>(light_userdata where_from)`  
where_from - the light userdata from which the float or double will be read  
returns a number with a value of the float/double read from memory that `where_from` points to  
### lu.readptr()
`light_userdata lu.readptr(light_userdata where_from)`  
where_from - the light userdata from which the pointer will be read  
returns a light userdata with the value read from memory that `where_from` points to  
### lu.readstring()
`string lu.readstring(light_userdata where_from)`  
where_from - the light userdata from which the null terminated C string will be read  
returns the string corresponding to the null terminated C string read from memory that `where_from` points to  
`string lu.readstring(light_userdata where_from, integer length)`  
where_from - the light userdata from which the string will be read  
length - how much characters needs to be read into the string  
returns the string corresponding to bytes starting from `where_from` and ending at `where_from+length` (unless a null character preceeds `where_from+length`)  
## cipherpatch
`cipherpatch` is a generic and relatively safe way to patch the game library code
### cipherpatch.new()
`CipherPatch cipherpatch.new`  
returns a new CipherPatch  
NOTE - you need to store this globally, if the patch gets garbage collected it will revert itself  
### CipherPatch:set_Opcode()
`void CipherPatch:set_Opcode(string opcode)`  
opcode - a string with an even amount of hexadecimal characters  
returns nothing, sets the replacement byte(s) for the patch  
### CipherPatch:set_Address()
`void CipherPatch:set_Address(light_userdata address, boolean is_local=true)`  
address - the start address where byte(s) would be replaced  
is_local - set to true if the address is relative to the library, false otherwise  
returns nothing, sets the address where the byte(s) would be replaced  
`void CipherPatch:set_Address(string symbol)`  
symbol - the symbol at the address of which the byte(s) would be replaced  
returns nothing, sets the address where the byte(s) would be replaced  
`void CipherPatch:set_Address(string pattern, string mask)` NOTE: USE AT YOUR OWN RISK! This function uses an irrelibale KittyMemory::find_from_lib API which is not guaranteed to fully scan the library  
  
pattern - the pattern, consisting of raw characters  
mask - the mask for the pattern  
Uses the same pattern scan algorithm as [CipherScan](#ciphercipherscan) to find the address  
returns nothing, sets the address where the byte(s) would be replaced to the result of the pattern scan  
### CipherPatch:set_libName()
`void CipherPatch:set_libName(string name)`  
name - the library name  
NOTE: must be called before set_Address  
returns nothing, sets the library from which the local adresses would be relative to and which would be scanned when using set_Address  
### CipherPatch:set_Lock()
`void CipherPatch:set_Lock(boolean lock)`  
lock - if this patch is locked or not  
If the patch is set to locked, no other CipherPatch can make a patch to this memory address  
returns nothing, sets the locked status of the patch  
### CipherPatch:Fire()
`void CipherPatch:Fire()`  
returns nothing, applies the patch  
### CipherPatch:Restore()
`void CipherPatch:Restore()`  
returns nothing, reverts the patch  
## dyncall
dyncall allows you to call any C(++) function by its pointer, if you know the arguments and the return value
### dyncall()
`mixed dyncall(<cfunction/number/light_userdata> function_ptr, string signature, ...)`  
function_ptr - the pointer to the function that needs to be called  
signature - a dyncall function signature, as described [in the dyncall manual](https://dyncall.org/docs/manual/manualse4.html)  
... - arguments for the function that needs to be called, as specified in the dyncall signature  
returns the result of the function call, with the type dictated by the dyncall signature  